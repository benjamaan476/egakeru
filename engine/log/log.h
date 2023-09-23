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


#define LOG_TRACE(...) spdlog::get(egkr::log::get_log_name().data())->trace(__VA_ARGS__)
#define LOG_INFO(...)  spdlog::get(egkr::log::get_log_name().data())->info(__VA_ARGS__)
#define LOG_WARN(...)  spdlog::get(egkr::log::get_log_name().data())->warn(__VA_ARGS__)
#define LOG_ERROR(...) spdlog::get(egkr::log::get_log_name().data())->error(__VA_ARGS__)
#define LOG_FATAL(...) spdlog::get(egkr::log::get_log_name().data())->critical(__VA_ARGS__)