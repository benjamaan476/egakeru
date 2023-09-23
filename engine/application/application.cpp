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

		state_.width = game->get_application_configuration().width;
		state_.height = game->get_application_configuration().height;
		state_.name = game->get_application_configuration().name;
		state_.game = std::move(game);

		//Init subsystems

		//
		const uint32_t start_x = 100;
		const uint32_t start_y = 100;

		const platform_configuration platform_config = { .start_x = start_x, .start_y = start_y, .width = state_.width, .height = state_.height, .name = state_.name };


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

			state_.game->update(delta_time);

			state_.game->render(delta_time);
			state_.renderer->draw_frame({ delta_time });

			auto frame_duration = state_.platform->get_time() - frame_time;
			if (limit_framerate_ && frame_duration < frame_time_)
			{
				auto time_remaining = frame_time_ - frame_duration;
				state_.platform->sleep(time_remaining - 1ms);
				LOG_INFO("Hi");
			}

			state_.is_running &= state_.platform->is_running();

			last_time_ = time;
		}
	}

	void application::shutdown()
	{
		state_.is_running = false;
	}

	bool application::on_event(event_code code, void* /*sender*/, void* /*listener*/, const event_context& context)
	{
		if (code == event_code::key_down)
		{
			const size_t array_size{ 8 };
			auto key = (egkr::key)std::get<std::array<int16_t, array_size>>(context)[0];

			switch (key)
			{
			case egkr::key::esc:
				shutdown();
				break;
			case egkr::key::unknown:
			case egkr::key::space:
			case egkr::key::apostrophe:
			case egkr::key::comma:
			case egkr::key::minus:
			case egkr::key::period:
			case egkr::key::slash:
			case egkr::key::key_0:
			case egkr::key::key_1:
			case egkr::key::key_2:
			case egkr::key::key_3:
			case egkr::key::key_4:
			case egkr::key::key_5:
			case egkr::key::key_6:
			case egkr::key::key_7:
			case egkr::key::key_8:
			case egkr::key::key_9:
			case egkr::key::semicolon:
			case egkr::key::equal:
			case egkr::key::a:
			case egkr::key::b:
			case egkr::key::c:
			case egkr::key::d:
			case egkr::key::e:
			case egkr::key::f:
			case egkr::key::g:
			case egkr::key::h:
			case egkr::key::i:
			case egkr::key::j:
			case egkr::key::k:
			case egkr::key::l:
			case egkr::key::m:
			case egkr::key::n:
			case egkr::key::o:
			case egkr::key::p:
			case egkr::key::q:
			case egkr::key::r:
			case egkr::key::s:
			case egkr::key::t:
			case egkr::key::u:
			case egkr::key::v:
			case egkr::key::w:
			case egkr::key::x:
			case egkr::key::y:
			case egkr::key::z:
			case egkr::key::left_bracket:
			case egkr::key::backslash:
			case egkr::key::right_bracket:
			case egkr::key::grave:
			case egkr::key::world_1:
			case egkr::key::world_2:
			case egkr::key::enter:
			case egkr::key::tab:
			case egkr::key::backspace:
			case egkr::key::insert:
			case egkr::key::del:
			case egkr::key::right:
			case egkr::key::left:
			case egkr::key::down:
			case egkr::key::up:
			case egkr::key::page_up:
			case egkr::key::page_down:
			case egkr::key::home:
			case egkr::key::end:
			case egkr::key::caps_lock:
			case egkr::key::scroll_lock:
			case egkr::key::num_lock:
			case egkr::key::print_screen:
			case egkr::key::pause:
			case egkr::key::f1:
			case egkr::key::f2:
			case egkr::key::f3:
			case egkr::key::f4:
			case egkr::key::f5:
			case egkr::key::f6:
			case egkr::key::f7:
			case egkr::key::f8:
			case egkr::key::f9:
			case egkr::key::f10:
			case egkr::key::f11:
			case egkr::key::f12:
			case egkr::key::f13:
			case egkr::key::f14:
			case egkr::key::f15:
			case egkr::key::f16:
			case egkr::key::f17:
			case egkr::key::f18:
			case egkr::key::f19:
			case egkr::key::f20:
			case egkr::key::f21:
			case egkr::key::f22:
			case egkr::key::f23:
			case egkr::key::f24:
			case egkr::key::f25:
			case egkr::key::kp_0:
			case egkr::key::kp_1:
			case egkr::key::kp_2:
			case egkr::key::kp_3:
			case egkr::key::kp_4:
			case egkr::key::kp_5:
			case egkr::key::kp_6:
			case egkr::key::kp_7:
			case egkr::key::kp_8:
			case egkr::key::kp_9:
			case egkr::key::kp_decimal:
			case egkr::key::kp_divide:
			case egkr::key::kp_multiply:
			case egkr::key::kp_substract:
			case egkr::key::kp_add:
			case egkr::key::kp_enter:
			case egkr::key::kp_equal:
			case egkr::key::left_shift:
			case egkr::key::left_control:
			case egkr::key::left_alt:
			case egkr::key::left_super:
			case egkr::key::right_shift:
			case egkr::key::right_control:
			case egkr::key::right_alt:
			case egkr::key::right_super:
			case egkr::key::menu:
			default:
				break;
			}
		}

		return false;
	}


}