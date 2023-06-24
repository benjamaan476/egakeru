#pragma once

#include "EngineCore.h"

#include <string_view>
#include <mutex>
#include <filesystem>
#include <fstream>

struct ProfileResult
{
	std::string name;
	std::chrono::duration<double, std::micro> start;
	std::chrono::microseconds elapsedTime;
	std::thread::id threadId;
};

struct InstrumentationSession
{
	std::string name;
};

class Instrumentor
{
public:
	Instrumentor(const Instrumentor&) = delete;
	Instrumentor(Instrumentor&&) = delete;

	void BeginSession(std::string_view name, const std::filesystem::path& filepath) noexcept
	{
		std::scoped_lock lock(mutex);
		if (currentSession)
		{
			if (Log::GetLogger())
			{
				LOG_ERROR("Instrumentor::BeginSession('{0}') when session '{1}' already open.", name, currentSession->name);
			}
			InternalEndSession();
		}
		outputStream.open(filepath.string());

		if (outputStream.is_open())
		{
			currentSession = new InstrumentationSession{ name.data() };
			WriteHeader();
		}
		else
		{
			if (Log::GetLogger())
			{
				LOG_ERROR("Instrumentor could not open results file '{0}'", filepath.string());
			}
		}
	}

	void EndSession() noexcept
	{
		std::scoped_lock lock(mutex);
		InternalEndSession();
	}

	void WriteProfile(const ProfileResult& result)
	{
		std::stringstream json;

		json << std::setprecision(3) << std::fixed;
		json << ",{";
		json << "\"cat\":\"function\",";
		json << "\"dur\":" << (result.elapsedTime.count()) << ',';
		json << "\"name\":\"" << result.name << "\",";
		json << "\"ph\":\"X\",";
		json << "\"pid\":0,";
		json << "\"tid\":" << result.threadId << ",";
		json << "\"ts\":" << result.start.count();
		json << "}";

		std::scoped_lock lock(mutex);
		if (currentSession)
		{
			outputStream << json.str();
			outputStream.flush();
		}
	}

	static Instrumentor& get()
	{
		static Instrumentor instance;
		return instance;
	}

private:

	Instrumentor() {}
	~Instrumentor()
	{
		EndSession();
	}

	void WriteHeader()
	{
		outputStream << "{\"otherData\": {}, \"traceEvents\":[{}";
		outputStream.flush();
	}

	void WriteFooter()
	{
		outputStream << "]}";
		outputStream.flush();
	}

	void InternalEndSession()
	{
		if (currentSession)
		{
			WriteFooter();
			outputStream.close();
			delete currentSession;
			currentSession = nullptr;
		}
	}

	std::mutex mutex;
	InstrumentationSession* currentSession{};
	std::ofstream outputStream{};
};

class InstrumentationTimer
{
public:
	InstrumentationTimer(std::string_view name)
		: _name(name)
	{
		startTimepoint = std::chrono::steady_clock::now();
	}

	~InstrumentationTimer()
	{
		if (!stopped)
		{
			Stop();
		}
	}

	void Stop()
	{
		auto endTimepoint = std::chrono::steady_clock::now();
		auto highResStart = std::chrono::duration<double, std::micro>{ startTimepoint.time_since_epoch() };
		auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(startTimepoint).time_since_epoch();

		Instrumentor::get().WriteProfile({_name.data(), highResStart, elapsedTime, std::this_thread::get_id()});
		stopped = true;
	}

private:

	std::string_view _name;
	std::chrono::time_point<std::chrono::steady_clock> startTimepoint;
	bool stopped{ false };
};

namespace InstrumentorUtils
{
	template<size_t N>
	struct ChangeResult
	{
		char data[N];
	};

	template <size_t N, size_t K>
	constexpr auto cleanupOutputString(const char(&expr)[N], const char(&remove)[K])
	{
		ChangeResult<N> result = {};

		auto srcIndex = 0;
		auto dstIndex = 0;

		while (srcIndex < N)
		{
			auto matchIndex = 0;
			while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
			{
				matchIndex++;
			}

			if (matchIndex == K - 1)
			{
				srcIndex += matchIndex;
			}

			result.data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
			srcIndex++;
		}
		return result;
	}
}

#define PROFILE 1

#if PROFILE
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define FUNC_SIG __func__
#else
#define FUNC_SIG "FUNC_SIG unknown!"
#endif


#define PROFILE_BEGIN_SESSION(name, filepath) Instrumentor::get().BeginSession(name, filepath);
#define PROFILE_END_SESSION() Instrumentor::get().EndSession();
#define PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = InstrumentorUtils::cleanupOutputString(name, "__cdecl"); \
										InstrumentationTimer timer##line(fixedName##line.data);
#define PROFILE_SCOPE_LINE(name, line) PROFILE_SCOPE_LINE2(name, line)
#define PROFILE_SCOPE(name) PROFILE_SCOPE_LINE(name, __LINE__)
#define PROFILE_FUNCTION() PROFILE_SCOPE(FUNC_SIG)

#else	
	#define PROFILE_BEGIN_SESSION(name, filepath)
	#define PROFILE_END_SESSION()
	#define PROFILE_SCOPE(name)
	#define PROFILE_FUNCTION()
#endif