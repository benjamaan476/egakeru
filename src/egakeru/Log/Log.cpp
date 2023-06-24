#include "Log/Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"


void Log::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");

	coreLogger = spdlog::stdout_color_mt("Engine");
	coreLogger->set_level(spdlog::level::trace);
}
