#include "application.h"
#include "input.h"

using namespace std::chrono_literals;

namespace egkr
{
	application::unique_ptr application::create(game::unique_ptr game)
	{
		if (!is_initialised_)
		{
			return std::make_unique<application>(std::move(game));
		}

		LOG_WARN("Application already initialised");
		return nullptr;
	}

	application::application(game::unique_ptr game)
	{

		state_.width_ = game->get_application_configuration().width_;
		state_.height_ = game->get_application_configuration().height_;
		state_.name = game->get_application_configuration().name;
		state_.game = std::move(game);

		//Init subsystems

		egkr::log::init();
		egkr::input::init();
		//
		const uint32_t start_x = 100;
		const uint32_t start_y = 100;

		const platform_configuration platform_config = { .start_x = start_x, .start_y = start_y, .width_ = state_.width_, .height_ = state_.height_, .name = state_.name };



		state_.platform = egkr::platform::create(egkr::platform_type::windows);
		if (state_.platform == nullptr)
		{
			LOG_FATAL("Failed to create platform");
			return;
		}

		auto success = state_.platform->startup(platform_config);
		if (!success)
		{
			LOG_FATAL("Failed to start platform");
			return;
		}

		state_.renderer = renderer_frontend::create(backend_type::vulkan, state_.platform);
		if (!state_.renderer->init())
		{
			LOG_FATAL("Failed to initialise renderer");
		}

		if (!state_.game->init())
		{
			LOG_ERROR("FAiled to create game");
		}

		event::register_event(event_code::key_down, nullptr, application::on_event);
		event::register_event(event_code::quit, nullptr, application::on_event);
		event::register_event(event_code::resize, nullptr, application::on_resize);
		state_.is_running = true;

	}

	void application::run()
	{
		while (state_.is_running)
		{
			auto time = state_.platform->get_time();
			auto delta_time = time - last_time_;

			auto frame_time = time;
			state_.platform->pump();

			if (!state_.is_suspended)
			{
				state_.game->update(delta_time);

				state_.game->render(delta_time);
				state_.renderer->draw_frame({ delta_time});
			}
			auto frame_duration = state_.platform->get_time() - frame_time;
			if (limit_framerate_ && frame_duration < frame_time_)
			{
				auto time_remaining = frame_time_ - frame_duration;
				state_.platform->sleep(time_remaining - 1ms);
				LOG_INFO("Hi");
			}

			last_time_ = time;
		}
	}

	void application::shutdown()
	{

		event::unregister_event(event_code::key_down, nullptr, on_event);
		event::unregister_event(event_code::quit, nullptr, on_event);
		event::unregister_event(event_code::resize, nullptr, application::on_resize);

		state_.renderer->shutdown();
		state_.platform->shutdown();
	}

	bool application::on_event(event_code code, void* /*sender*/, void* /*listener*/, const event_context& context)
	{
		if (code == event_code::quit)
		{
		state_.is_running = false;
		}

		if (code == event_code::key_down)
		{
			const size_t array_size{ 8 };
			auto key = (egkr::key)std::get<std::array<int16_t, array_size>>(context)[0];

			switch (key)
			{
			case egkr::key::esc:
				state_.is_running = false;
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

			if (state_.width_ != (uint32_t)width || state_.height_ != (uint32_t)height)
			{
				state_.width_ = (uint32_t)width;
				state_.height_ = (uint32_t)height;

				if (width == 0 && height == 0)
				{
					state_.is_suspended = true;
					return false;
				}

				if (state_.is_suspended)
				{
					state_.is_suspended = false;
				}


				state_.game->resize(width, height);
				state_.renderer->on_resize(width, height);

			}
		}
		return false;
	}

}