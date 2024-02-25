#include "input.h"
#include "event.h"

namespace egkr
{

	static input::unique_ptr state{};

	system* input::create()
	{
		state = std::make_unique<input>();
		return state.get();
	}


	bool input::init()
	{
		LOG_INFO("Initialised input");
		return true;
	}

	bool input::update(float /*delta_time*/ )
	{
		previous_keyboard = current_keyboard;
		previous_mouse = current_mouse;
		return true;
	}

	bool input::shutdown()
	{
		if (state)
		{
			state.release();
			return true;
		}
		LOG_WARN("Input system already shutdown. Don't do this twice");
		return false;
	}

	bool input::is_key_down(key key)
	{
		return state->current_keyboard.keys[(int16_t)key];
	}

	bool input::is_key_up(key key)
	{
		return !is_key_down(key);
	}

	bool input::was_key_down(key key)
	{
		return state->previous_keyboard.keys[(int16_t)key];
	}

	bool input::was_key_up(key key)
	{
		return !was_key_down(key);
	}

	bool input::was_key_pressed(key key)
	{
		return was_key_up(key) && is_key_down(key);
	}

	bool input::was_key_released(key key)
	{
		return was_key_down(key) && is_key_up(key);
	}

	void input::process_key(key key, bool pressed)
	{
		if (key == key::unknown)
		{
			return;
		}

		auto& key_state = state->current_keyboard.keys[(int16_t)key];

		if (key_state != pressed)
		{
			key_state = pressed;

			auto code = pressed ? event_code::key_down : event_code::key_up;
			event_context context{};
			const int array_size{ 8 };
			context.context_ = std::array<int16_t, array_size>{ (int16_t)key };

			event::fire_event(code, nullptr, context);
		}
	}

	bool input::is_button_up(mouse_button button)
	{
		return !is_button_down(button);
	}

	bool input::is_button_down(mouse_button button)
	{
		return state->current_mouse.buttons[(int16_t)button];
	}

	bool input::was_button_up(mouse_button button)
	{
		return !was_button_down(button);
	}

	bool input::was_button_down(mouse_button button)
	{
		return state->previous_mouse.buttons[(int16_t)button];
	}

	void input::process_button(mouse_button button, bool pressed)
	{
		auto& button_state = state->current_mouse.buttons[(int16_t)button];

		if (button_state != pressed)
		{
			button_state = pressed;

			auto code = pressed ? event_code::mouse_down : event_code::mouse_up;
			event_context context{};
			const int array_size{ 8 };
			context.context_ = std::array<int16_t, array_size>{ (int16_t)button };
			event::fire_event(code, nullptr, context);
		}

	}

	int2 input::get_mouse_position()
	{
		return { 1 ,1 };
	}

	int2 input::get_previous_mouse_position()
	{
		return { 0, 0 };
	}

	void input::process_mouse_move()
	{
	}

	void input::process_mouse_wheel()
	{
	}
}