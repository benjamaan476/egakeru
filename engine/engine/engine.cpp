#include "engine.h"
#include "input.h"

#include "systems/view_system.h"
#include "systems/audio_system.h"
#include "renderer/renderer_frontend.h"

using namespace std::chrono_literals;

namespace egkr
{
	static engine::unique_ptr engine_;
	bool engine::create(application::unique_ptr application)
	{
		if (!engine_)
		{
			engine_ = std::make_unique<engine>(std::move(application));
			return true;
		}

		LOG_WARN("engine already initialised");
		return false;
	}

	engine::engine(application::unique_ptr application)
		: name_{ application->get_engine_configuration().name }
	{
		application_ = std::move(application);

		egkr::log::init();

		const uint32_t start_x = 100;
		const uint32_t start_y = 100;

		const platform::configuration platform_config = 
		{ 
			.start_x = start_x, 
			.start_y = start_y,
			.width_ = application_->get_engine_configuration().width,
			.height_ = application_->get_engine_configuration().height,
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

		system_manager::create(application_.get());

		if (!renderer->init())
		{
			LOG_FATAL("Failed to initialise renderer");
		}

		system_manager::init();

		application_->boot();

		audio::system_configuration audio_configuration{ .audio_channel_count = 8 };
		audio::audio_system::create(audio_configuration);

		application_->set_engine(this);

		if (!application_->init())
		{
			LOG_ERROR("FAiled to create application");
		}

		std::ranges::for_each(application_->get_render_view_configuration(), [](const auto& config) { view_system::create_view(config); });
		event::register_event(event_code::key_down, nullptr, engine::on_event);
		event::register_event(event_code::quit, nullptr, engine::on_event);
		event::register_event(event_code::resize, nullptr, engine::on_resize);

		is_running_ = true;
		is_initialised_ = true;
	}

	void engine::run()
	{
		while (engine_->is_running_)
		{
			auto time = engine_->platform_->get_time();
			std::chrono::duration<double, std::ratio<1, 1>> delta = time - engine_->last_time_;
			auto delta_time = delta.count();

			auto frame_time = time;
			engine_->platform_->pump();

			if (!engine_->is_suspended_)
			{
				system_manager::update(delta_time);
				engine_->application_->update(delta_time);

				render_packet packet{};

				engine_->application_->render(&packet, delta_time);

				renderer->draw_frame(packet);
			}
			auto frame_duration = engine_->platform_->get_time() - frame_time;
			if (engine_->limit_framerate_ && frame_duration < engine_->frame_time_)
			{
				auto time_remaining = engine_->frame_time_ - frame_duration;
				engine_->platform_->sleep(time_remaining);
			}

			system_manager::update_input();
			engine_->last_time_ = time;
		}
	}

	void engine::shutdown()
	{
		engine_->application_->shutdown();
		egkr::event::unregister_event(egkr::event_code::key_down, nullptr, on_event);
		egkr::event::unregister_event(egkr::event_code::quit, nullptr, on_event);
		egkr::event::unregister_event(egkr::event_code::resize, nullptr, engine::on_resize);

		system_manager::shutdown();
		renderer->shutdown();
		engine_->platform_->shutdown();
	}

	bool engine::on_event(event_code code, void* /*sender*/, void* /*listener*/, const event_context& context)
	{
		if (code == event_code::quit)
		{
			engine_->is_running_ = false;
		}

		if (code == event_code::key_down)
		{
			uint16_t key_value{};
			context.get(0, key_value);

			switch ((key)key_value)
			{
			case egkr::key::esc:
				engine_->is_running_ = false;
				break;
			default:
				break;
			}
		}

		return false;
	}

	bool engine::on_resize(event_code code, void* /*sender*/, void* /*listener*/, const event_context& context)
	{
		if (code == event_code::resize)
		{
			int32_t width{};
			context.get(0, width);
			int32_t height{};
			context.get(1, height);

			if (engine_->application_->resize(width, height))
			{
				if (width == 0 && height == 0)
				{
					engine_->is_suspended_ = true;
					return false;
				}

				if (engine_->is_suspended_)
				{
					engine_->is_suspended_ = false;
				}

				renderer->on_resize(width, height);
			}
		}
		return false;
	}
}