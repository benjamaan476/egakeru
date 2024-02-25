#include "application.h"
#include "input.h"

#include "systems/shader_system.h"
#include "systems/view_system.h"
#include "systems/light_system.h"
#include "systems/font_system.h"
#include "systems/audio_system.h"

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

		renderer_frontend::create(backend_type::vulkan, platform_);

		system_manager::create();


		if (!view_system::create(renderer.get()))
		{
			LOG_FATAL("Failed to create view system");
			return;
		}

		if (!light_system::create())
		{
			LOG_FATAL("Failed to create light system");
			return;
		}

		if (!renderer->init())
		{
			LOG_FATAL("Failed to initialise renderer");
		}

		system_manager::init();


		game_->boot();

		if (!font_system::create(renderer.get(), game_->get_font_system_configuration()))
		{
			LOG_FATAL("Failed to create font system");
		}

		audio::system_configuration audio_configuration{ .audio_channel_count = 8 };
		audio::audio_system::create(audio_configuration);

		font_system::init();
		view_system::init();
		light_system::init();

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
				system_manager::update(delta_time);
				application_->game_->update(delta_time);

				render_packet packet{};

				application_->game_->render(&packet, delta_time);

				renderer->draw_frame(packet);
			}
			auto frame_duration = application_->platform_->get_time() - frame_time;
			if (application_->limit_framerate_ && frame_duration < application_->frame_time_)
			{
				auto time_remaining = application_->frame_time_ - frame_duration;
				application_->platform_->sleep(time_remaining);
			}

			system_manager::update_input();
			application_->last_time_ = time;
		}
	}

	void application::shutdown()
	{
		application_->game_->shutdown();
		egkr::event::unregister_event(egkr::event_code::key_down, nullptr, on_event);
		egkr::event::unregister_event(egkr::event_code::quit, nullptr, on_event);
		egkr::event::unregister_event(egkr::event_code::resize, nullptr, application::on_resize);

		system_manager::shutdown();
		light_system::shutdown();
		view_system::shutdown();
		renderer->shutdown();
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
			int16_t key_value{};
			context.get(0, key_value);

			switch ((key)key_value)
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
			uint32_t width{};
			context.get(0, width);
			uint32_t height{};
			context.get(1, height);

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

				renderer->on_resize(width, height);
			}
		}
		return false;
	}
}