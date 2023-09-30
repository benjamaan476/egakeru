#include "log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace egkr
{
	void log::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		core_logger_ = spdlog::stdout_color_mt(log_name_.data());
		core_logger_->set_level(spdlog::level::trace);

		core_logger_ = spdlog::get(get_log_name().data());
	}
}