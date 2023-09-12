#include "log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace egkr
{
	void log::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		core_logger_ = spdlog::stdout_color_mt("Engine");
		core_logger_->set_level(spdlog::level::trace);
	}
}