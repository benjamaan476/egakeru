#include "pch.h"

#include "entry.h"
#include "sandbox_application.h"

#include "renderer/renderer_frontend.h"

#include "../plugins/renderer/vulkan/vulkan_main.h"
#include "../plugins/renderer/vulkan/renderer_vulkan.h"
#include <iostream>

egkr::application::unique_ptr create_application()
{
	std::cout << vulkan_test() << std::endl;
	const uint32_t width_{ 800 };
	const uint32_t height_{ 600 };
	const std::string name{"sandbox"};
	const auto application_config = egkr::engine_configuration{ .width = width_, .height = height_, .name = name };

	auto game = std::make_unique<sandbox_application>(application_config);
	return game;
}

void initialise_renderer_backend(egkr::renderer_frontend* frontend)
{
	frontend->backend_ = egkr::renderer_vulkan::create(frontend->platform_);
}