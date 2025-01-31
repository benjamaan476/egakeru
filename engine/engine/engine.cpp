#include "engine.h"

using namespace std::chrono_literals;

namespace egkr
{
	static engine::unique_ptr engine_;
	const engine::unique_ptr& engine::get() { return engine_; }

	bool engine::create(application::unique_ptr application)
	{
		if (!engine_)
		{
			engine_ = std::make_unique<engine>(std::move(application));
			engine_->init();
			return true;
		}

		LOG_WARN("engine already initialised");
		return false;
	}

	engine::engine(application::unique_ptr application) : name_{ application->get_engine_configuration().name }, application_(std::move(application))
	{
	}

	void engine::init()
	{
		egkr::log::init();
		const uint32_t start_x = 100;
		const uint32_t start_y = 100;

		const platform::configuration platform_config{ .start_x = start_x, .start_y = start_y, .width_ = application_->get_engine_configuration().width, .height_ = application_->get_engine_configuration().height, .name = name_ };
		platform_ = egkr::platform::create(platform_config);

		if (platform_ == nullptr)
		{
			LOG_FATAL("Failed to create platform");
			return;
		}

		renderer_ = renderer_frontend::create(application_->move_renderer_plugin());
		if (!renderer_->init(platform_))
		{
			LOG_FATAL("Failed to initialise renderer");
		}

		application_->set_engine(this);
		system_manager::create(application_.get());


		application_->boot();

		system_manager::init();



		if (!application_->init())
		{
			LOG_ERROR("FAiled to create application");
		}

		event::register_event(event::code::key_down, nullptr, engine::on_event);
		event::register_event(event::code::quit, nullptr, engine::on_event);
		event::register_event(event::code::resize, nullptr, engine::on_resize);

		is_running_ = true;
		is_initialised_ = true;
	}

	void engine::run()
	{
		while (engine_->is_running_)
		{
			auto time = engine_->platform_->get_time();
			std::chrono::duration<double, std::ratio<1, 1>> delta = time - engine_->last_time_;
			engine_->frame_data_.delta_time = (float)delta.count();
			engine_->frame_data_.total_time = (double)time.count();
			engine_->platform_->pump();

			if (!engine_->is_suspended_)
			{
				system_manager::update(engine_->frame_data_);
				engine_->application_->update(engine_->frame_data_);

				engine_->application_->prepare_frame(engine_->frame_data_);
				engine_->application_->render_frame(engine_->frame_data_);
			}

			auto frame_duration = engine_->platform_->get_time() - time;
			if (engine_->limit_framerate_ && frame_duration < engine_->frame_time_)
			{
				auto time_remaining = engine_->frame_time_ - frame_duration;
				engine_->platform_->sleep(time_remaining);
			}

			system_manager::update_input(engine_->frame_data_);
			engine_->last_time_ = time;
		}
	}

	void engine::shutdown()
	{
		engine_->get_renderer()->tidy_up();
		engine_->application_->shutdown();
		egkr::event::unregister_event(egkr::event::code::key_down, nullptr, on_event);
		egkr::event::unregister_event(egkr::event::code::quit, nullptr, on_event);
		egkr::event::unregister_event(egkr::event::code::resize, nullptr, on_resize);

		system_manager::shutdown();
		engine_->get_renderer()->shutdown();
		engine_->platform_->shutdown();
	}

	const frame_data& engine::get_frame_data()
	{
		return engine_->frame_data_;
	}

	bool engine::on_event(event::code code, void* /*sender*/, void* /*listener*/, const event::context& context)
	{
		if (code == event::code::quit)
		{
			engine_->is_running_ = false;
		}

		if (code == event::code::key_down)
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

	bool engine::on_resize(event::code code, void* /*sender*/, void* /*listener*/, const event::context& context)
	{
		if (code == event::code::resize)
		{
			uint32_t width{};
			context.get(0, width);
			uint32_t height{};
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

				engine_->get_renderer()->on_resize(width, height);
			}
		}
		return false;
	}
}
