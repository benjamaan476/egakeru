#include "pch.h"

#include "entry.h"
#include "plugins/renderer/vulkan/renderer_vulkan.h"
#include "sandbox_application.h"

egkr::application::unique_ptr create_application()
{
	const uint32_t width_{ 800 };
	const uint32_t height_{ 600 };
	const std::string name{"sandbox"};
	const auto application_config = egkr::engine_configuration{ .width = width_, .height = height_, .name = name };

	auto renderer_plugin = egkr::renderer_vulkan::create();
	auto game = std::make_unique<sandbox_application>(application_config, std::move(renderer_plugin));
	return game;
}
