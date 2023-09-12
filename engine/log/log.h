#pragma once

#include <spdlog/spdlog.h>

namespace egkr
{
	class log
	{
	public:
		__declspec(dllexport) static void init();

	private:

		inline static std::shared_ptr<spdlog::logger> core_logger_;
	};
}
#define LOG_TRACE(...) spdlog::get("Engine")->trace(__VA_ARGS__)
#define LOG_INFO(...) spdlog::get("Engine")->info(__VA_ARGS__)
#define LOG_WARN(...) spdlog::get("Engine")->warn(__VA_ARGS__)
#define LOG_ERROR(...) spdlog::get("Engine")->error(__VA_ARGS__)
#define LOG_FATAL(...) spdlog::get("Engine")->critical(__VA_ARGS__)
