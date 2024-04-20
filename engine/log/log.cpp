#include "log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace egkr
{
	void log::init(const std::string& logger_name)
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		core_logger_ = spdlog::stdout_color_mt(logger_name);
		core_logger_->set_level(spdlog::level::trace);

		core_logger_ = spdlog::get(logger_name);
	}
}