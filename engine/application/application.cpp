#include "application.h"
#include "input.h"

#include "systems/resource_system.h"
#include "systems/texture_system.h"
#include "systems/material_system.h"
#include "systems/geometry_system.h"
#include "systems/shader_system.h"
#include "systems/camera_system.h"
#include "systems/view_system.h"
#include "systems/light_system.h"
#include "systems/job_system.h"
#include "systems/font_system.h"

#include "resources/transform.h"
#include "resources/geometry.h"

using namespace std::chrono_literals;

namespace egkr
{
	static application::unique_ptr application_;
	bool application::create(game::unique_ptr game)
	{
		if (!application_)
		{
			application_ = std::make_unique<application>(std::move(game));
			return true;
		}

		LOG_WARN("Application already initialised");
		return false;
	}

	application::application(game::unique_ptr game)
	{
		name_ = game->get_application_configuration().name;
		game_ = std::move(game);

		//Init subsystems
		egkr::log::init();
		egkr::input::init();
		//
		const uint32_t start_x = 100;
		const uint32_t start_y = 100;

		const platform_configuration platform_config = 
		{ 
			.start_x = start_x, 
			.start_y = start_y,
			.width_ = game_->get_application_configuration().width,
			.height_ = game_->get_application_configuration().height,
			.name = name_
		};

		platform_ = egkr::platform::create(egkr::platform_type::windows);
		if (platform_ == nullptr)
		{
			LOG_FATAL("Failed to create platform");
			return;
		}

		auto success = platform_->startup(platform_config);
		if (!success)
		{
			LOG_FATAL("Failed to start platform");
			return;
		}

		renderer_ = renderer_frontend::create(backend_type::vulkan, platform_);

		resource_system_configuration resource_system_configuration{};
		resource_system_configuration.max_loader_count = 10;
		resource_system_configuration.base_path = "../../../../assets/";

		if (!resource_system::create(resource_system_configuration))
		{
			LOG_FATAL("Failed to create resource system");
		}
		resource_system::init();

		texture_system::create(renderer_.get(), { 1024 });
		if (!material_system::create(renderer_.get()))
		{
			LOG_FATAL("Failed to create material system");
		}

		if (!geometry_system::create(renderer_.get()))
		{
			LOG_FATAL("Failed to create geometry system");
		}

		shader_system_configuration shader_system_configuration{};
		shader_system_configuration.max_global_textures = 31;
		shader_system_configuration.max_instance_textures = 31;
		shader_system_configuration.max_shader_count = 1024;
		shader_system_configuration.max_uniform_count = 128;

		if (!shader_system::create(renderer_.get(), shader_system_configuration))
		{
			LOG_FATAL("Failed to create shader system");
		}


		if (!camera_system::create(renderer_.get(), { 31 }))
		{
			LOG_FATAL("Failed to create camera system");
		}

		if (!view_system::create(renderer_.get()))
		{
			LOG_FATAL("Failed to create view system");
			return;
		}

		if (!light_system::create())
		{
			LOG_FATAL("Failed to create light system");
			return;
		}

		if (!renderer_->init())
		{
			LOG_FATAL("Failed to initialise renderer");
		}

		uint8_t thread_count = 5;
		auto is_multi = renderer_->get_backend()->is_multithreaded();

		game_->boot();

		if (!font_system::create(renderer_.get(), game_->get_font_system_configuration()))
		{
			LOG_FATAL("Failed to create font system");
		}


		std::vector<job::type> types{thread_count};

		std::fill(types.begin(), types.end(), job::type::general);
		if (thread_count == 1 || !is_multi)
		{
			types[0] |= job::type::gpu_resource | job::type::resource_load;
		}
		else if(thread_count == 2)
		{
			types[0] |= job::type::gpu_resource;
			types[1] |= job::type::resource_load;
		}
		else
		{
			types[0] = job::type::gpu_resource;
			types[1] = job::type::resource_load;
		}

		if (!job_system::job_system::create({ .thread_count = thread_count, .type_masks = types }))
		{
			LOG_FATAL("Failed to create job system");
		}

		shader_system::init();
		texture_system::init();
		material_system::init();
		geometry_system::init();
		font_system::init();
		camera_system::init();
		view_system::init();
		light_system::init();
		job_system::job_system::init();

		game_->set_application(this);

		if (!game_->init())
		{
			LOG_ERROR("FAiled to create game");
		}

		std::ranges::for_each(game_->get_render_view_configuration(), [](const auto& config) { view_system::create_view(config); });
		event::register_event(event_code::key_down, nullptr, application::on_event);
		event::register_event(event_code::quit, nullptr, application::on_event);
		event::register_event(event_code::resize, nullptr, application::on_resize);

		is_running_ = true;
		is_initialised_ = true;
	}

	void application::run()
	{
		while (application_->is_running_)
		{
			auto time = application_->platform_->get_time();
			std::chrono::duration<double, std::ratio<1, 1>> delta = time - application_->last_time_;
			auto delta_time = delta.count();

			auto frame_time = time;
			application_->platform_->pump();

			if (!application_->is_suspended_)
			{
				job_system::job_system::update();
				application_->game_->update(delta_time);

				render_packet packet{};

				application_->game_->render(&packet, delta_time);

				application_->renderer_->draw_frame(packet);
			}
			auto frame_duration = application_->platform_->get_time() - frame_time;
			if (application_->limit_framerate_ && frame_duration < application_->frame_time_)
			{
				auto time_remaining = application_->frame_time_ - frame_duration;
				application_->platform_->sleep(time_remaining);
			}

			application_->last_time_ = time;
		}
	}

	void application::shutdown()
	{
		application_->game_->shutdown();
		egkr::event::unregister_event(egkr::event_code::key_down, nullptr, on_event);
		egkr::event::unregister_event(egkr::event_code::quit, nullptr, on_event);
		egkr::event::unregister_event(egkr::event_code::resize, nullptr, application::on_resize);

		light_system::shutdown();
		view_system::shutdown();
		shader_system::shutdown();
		texture_system::shutdown();
		material_system::shutdown();
		geometry_system::shutdown();
		application_->renderer_->shutdown();
		job_system::job_system::shutdown();
		application_->platform_->shutdown();
	}

	bool application::on_event(event_code code, void* /*sender*/, void* /*listener*/, const event_context& context)
	{
		if (code == event_code::quit)
		{
			application_->is_running_ = false;
		}

		if (code == event_code::key_down)
		{
			const size_t array_size{ 8 };
			auto key = (egkr::key)std::get<std::array<int16_t, array_size>>(context)[0];

			switch (key)
			{
			case egkr::key::esc:
				application_->is_running_ = false;
				break;
			default:
				break;
			}
		}

		return false;
	}

	bool application::on_resize(event_code code, void* /*sender*/, void* /*listener*/, const event_context& context)
	{
		if (code == event_code::resize)
		{
			const auto& context_array = std::get<std::array<int32_t, 4>>(context);
			const auto& width = context_array[0];
			const auto& height = context_array[1];

			if (application_->game_->resize(width, height))
			{
				if (width == 0 && height == 0)
				{
					application_->is_suspended_ = true;
					return false;
				}

				if (application_->is_suspended_)
				{
					application_->is_suspended_ = false;
				}

				application_->renderer_->on_resize(width, height);
			}
		}
		return false;
	}
}