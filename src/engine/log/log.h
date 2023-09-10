#pragma once

#include <spdlog/spdlog.h>

namespace egkr
{
	class log
	{
	public:
		static void init();

		inline static std::shared_ptr<spdlog::logger>& get_logger()
		{
			return core_logger_;
		}

	private:

		inline static std::shared_ptr<spdlog::logger> core_logger_;
	};
}
template<typename ...T>
constexpr auto LOG_TRACE(T&&... args) { return egkr::log::get_logger()->trace(std::forward<T>(args)...); }
template<typename ...T>
constexpr auto LOG_INFO(T&&... args) { return egkr::log::get_logger()->info(std::forward<T>(args)...); }
template<typename ...T>
constexpr auto LOG_WARN(T&& ... args) { return egkr::log::get_logger()->warn(std::forward<T>(args)...); }
template<typename ...T>
constexpr auto LOG_ERROR(T&&... args) { return egkr::log::get_logger()->error(std::forward<T>(args)...); }
template<typename ...T>
constexpr auto LOG_FATAL(T&&... args) { return egkr::log::get_logger()->critical(std::forward<T>(args)...); }
