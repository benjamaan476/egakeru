#include "pch.h"

#include "entry.h"
#include "plugins/renderer/vulkan/renderer_vulkan.h"
#include "sandbox_application.h"

// using PFN_renderer_create = egkr::renderer_backend::unique_ptr (*)();

egkr::application::unique_ptr create_application()
{
    const uint32_t width_{1080};
    const uint32_t height_{720};
    const std::string name{"sandbox"};
    const auto application_config = egkr::engine_configuration{.width = width_, .height = height_, .name = name};

    // auto renderer_library = egkr::platform::load_library("renderer_vulkan");
    // egkr::platform::load_function("_ZN4egkr15renderer_vulkan6createEv", renderer_library.value());
    //
    // egkr::renderer_backend::unique_ptr renderer_plugin;
    // for(const auto& function : renderer_library->functions)
    // {
    // 	if(function.name == "_ZN4egkr15renderer_vulkan6createEv")
    // 	{
    // 		renderer_plugin = ((PFN_renderer_create)function.pfn)();
    // 	}
    // }

    auto renderer_plugin = egkr::renderer_vulkan::create();
    auto game = std::make_unique<sandbox_application>(application_config, std::move(renderer_plugin));
    return game;
}
