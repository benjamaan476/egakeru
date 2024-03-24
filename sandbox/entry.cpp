#include "pch.h"

#include "entry.h"
#include "sandbox_application.h"

egkr::application::unique_ptr create_application()
{
	const uint32_t width_{ 800 };
	const uint32_t height_{ 600 };
	const std::string name{"sandbox"};
	const auto application_config = egkr::engine_configuration{ .width = width_, .height = height_, .name = name };

	auto game = std::make_unique<sandbox_application>(application_config);
	return game;
}