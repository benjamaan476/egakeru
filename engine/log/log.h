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
		static spdlog::logger* get_logger() { return core_logger_.get(); }
	private:
        constexpr static std::string_view log_name_{ "engine"sv };
		inline static std::shared_ptr<spdlog::logger> core_logger_{};
	};
}

#define LOG_TRACE(...) egkr::log::get_logger()->trace(__VA_ARGS__)
#define LOG_INFO(...)  egkr::log::get_logger()->info(__VA_ARGS__)
#define LOG_WARN(...)  egkr::log::get_logger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) egkr::log::get_logger()->error(__VA_ARGS__)
#define LOG_FATAL(...) egkr::log::get_logger()->critical(__VA_ARGS__)