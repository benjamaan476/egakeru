#include "system.h"

#include <systems/input.h>
#include <systems/resource_system.h>
#include <systems/texture_system.h>
#include <systems/material_system.h>
#include <systems/geometry_system.h>
#include <systems/job_system.h>
#include <systems/shader_system.h>
#include <systems/camera_system.h>
#include <systems/light_system.h>
#include <systems/font_system.h>
#include <systems/console_system.h>
#include <systems/evar_system.h>
#include <systems/audio_system.h>

#include <renderer/renderer_frontend.h>

#include <application/application.h>

namespace egkr
{
    static std::unique_ptr<system_manager> system_manager_state{};

    void system_manager::create(application* application)
    {
	if (system_manager_state)
	{
	    LOG_WARN("System manager already initialised");
	    return;
	}
	system_manager_state = std::make_unique<system_manager>(application);
    }

    system_manager::system_manager(application* application)
    {
	register_known(application);
	register_extension();
	register_user();
    }

    bool system_manager::init()
    {
	if (!system_manager_state)
	{
	    LOG_ERROR("System manager not created. Failed to init");
	    return false;
	}

	for (auto system : system_manager_state->registered_systems_ | std::views::values)
	{
	    if (!system->init())
	    {
		return false;
	    }
	}
	return true;
    }

    bool system_manager::update(const frame_data& frame_data)
    {
	for (auto& [type, system] : system_manager_state->registered_systems_)
	{
	    if (type == system_type::input)
	    {
		continue;
	    }

	    if (!system->update(frame_data))
	    {
		return false;
	    }
	}
	return true;
    }

    void system_manager::update_input(const frame_data& frame_data) { system_manager_state->registered_systems_[system_type::input]->update(frame_data); }

    void system_manager::shutdown()
    {
	shutdown_user();
	shutdown_extension();
	shutdown_known();

	if (system_manager_state)
	{
	    system_manager_state.reset();
	}
    }

    void system_manager::register_known(application* application)
    {
	registered_systems_.emplace(system_type::input, input::create());
	{
	    const resource_system::configuration resource_system_configuration{
	        .max_loader_count = 10,
	        .base_path = "../../../../assets/",
	    };

	    registered_systems_.emplace(system_type::resource, resource_system::create(resource_system_configuration));
	}
	{
	    registered_systems_.emplace(system_type::texture, texture_system::create({1024}));
	}
	{
	    registered_systems_.emplace(system_type::material, material_system::create());
	}
	{
	    registered_systems_.emplace(system_type::geometry, geometry_system::create());
	}
	{
	    const uint8_t thread_count = 5;
	    std::vector<job::type> types{thread_count};

	    std::fill(types.begin(), types.end(), job::type::general);
	    auto is_multi = renderer->get_backend()->is_multithreaded();
	    if (thread_count == 1 || !is_multi)
	    {
		types[0] |= job::type::gpu_resource | job::type::resource_load;
	    }
	    else if (thread_count == 2)
	    {
		types[0] |= job::type::gpu_resource;
		types[1] |= job::type::resource_load;
	    }
	    else
	    {
		types[0] = job::type::gpu_resource;
		types[1] = job::type::resource_load;
	    }

	    const job_system::configuration configuration{.thread_count = thread_count, .type_masks = types};
	    registered_systems_.emplace(system_type::job, job_system::create(configuration));
	}
	{
	    const shader_system::configuration configuration{
	        .max_shader_count = 1024,
	        .max_uniform_count = 128,
	        .max_global_textures = 31,
	        .max_instance_textures = 31,
	    };

	    registered_systems_.emplace(system_type::shader, shader_system::create(configuration));
	}
	{
	    const camera_system::configuration configuration{.max_registered_cameras = 31};
	    registered_systems_.emplace(system_type::camera, camera_system::create(configuration));
	}
	{
	    registered_systems_.emplace(system_type::light, light_system::create());
	}

	{
	    registered_systems_.emplace(system_type::font, font_system::create(application->get_font_system_configuration()));
	}
	{
	    registered_systems_.emplace(system_type::console, console::create());
	}
	{
	    registered_systems_.emplace(system_type::evar, evar_system::create());
	}
	{
	    audio::system_configuration audio_configuration{.audio_channel_count = 8};

	    registered_systems_.emplace(system_type::audio, audio::audio_system::create(audio_configuration));
	}
    }

    void system_manager::register_extension() { }

    void system_manager::register_user() { }

    void system_manager::shutdown_extension() { }

    void system_manager::shutdown_user() { }

    void system_manager::shutdown_known()
    {
	if (!system_manager_state)
	{
	    return;
	}
	system_manager_state->registered_systems_[system_type::audio]->shutdown();
	system_manager_state->registered_systems_[system_type::evar]->shutdown();
	system_manager_state->registered_systems_[system_type::console]->shutdown();
	system_manager_state->registered_systems_[system_type::font]->shutdown();
	system_manager_state->registered_systems_[system_type::light]->shutdown();
	system_manager_state->registered_systems_[system_type::render_view]->shutdown();
	system_manager_state->registered_systems_[system_type::camera]->shutdown();
	system_manager_state->registered_systems_[system_type::shader]->shutdown();
	system_manager_state->registered_systems_[system_type::job]->shutdown();
	system_manager_state->registered_systems_[system_type::geometry]->shutdown();
	system_manager_state->registered_systems_[system_type::material]->shutdown();
	system_manager_state->registered_systems_[system_type::texture]->shutdown();
	system_manager_state->registered_systems_[system_type::resource]->shutdown();
	system_manager_state->registered_systems_[system_type::input]->shutdown();
    }
}
