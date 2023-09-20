#pragma once
#include "../defines.h"

#include <spdlog/spdlog.h>
#include <string_view>

using namespace std::literals;

namespace egkr
{
	class log
	{
	public:
		API static void init();
        constexpr static std::string_view get_log_name() { return log_name_; }
	private:
        constexpr static std::string_view log_name_{ "engine"sv };
		inline static std::shared_ptr<spdlog::logger> core_logger_;
	};
}


template<typename ...T>
constexpr auto LOG_TRACE(T&&... args) { spdlog::get(egkr::log::get_log_name().data())->trace(std::forward<T>(args)...); }
template<typename ...T>
constexpr auto LOG_INFO(T&&... args) { spdlog::get(egkr::log::get_log_name().data())->info(std::forward<T>(args)...); }
template<typename ...T>
constexpr auto LOG_WARN(T&&... args) { spdlog::get(egkr::log::get_log_name().data())->warn(std::forward<T>(args)...); }
template<typename ...T>
constexpr auto LOG_ERROR(T&&... args) { spdlog::get(egkr::log::get_log_name().data())->error(std::forward<T>(args)...); }
template<typename ...T>
constexpr auto LOG_FATAL(T&&... args) { spdlog::get(egkr::log::get_log_name().data())->critical(std::forward<T>(args)...); }
