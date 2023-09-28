#include "input.h"
#include "event.h"

namespace egkr
{
	struct keyboard_state
	{
		std::array<bool, (size_t)key::key_count> keys{};
	};
	
	struct mouse_state
	{
		uint32_t x{};
		uint32_t y{};
		std::array<bool, (size_t)mouse_button::button_count> buttons{};
	};

	struct input_state
	{
		keyboard_state current_keyboard{};
		keyboard_state previous_keyboard{};

		mouse_state current_mouse{};
		mouse_state previous_mouse{};
	};

	static input_state state{};

	bool input::init()
	{
		LOG_INFO("Initialised input");
		return true;
	}

	void input::update()
	{
		state.previous_keyboard = state.current_keyboard;
		state.previous_mouse = state.current_mouse;
	}

	bool input::is_key_down(key key)
	{
		return state.current_keyboard.keys[(int16_t)key];
	}

	bool input::is_key_up(key key)
	{
		return !is_key_down(key);
	}

	bool input::was_key_up(key key)
	{
		return state.previous_keyboard.keys[(int16_t)key];
	}

	bool input::was_key_down(key key)
	{
		return !was_key_up(key);
	}

	void input::process_key(key key, bool pressed)
	{
		auto& key_state = state.current_keyboard.keys[(int16_t)key];

		if (key_state != pressed)
		{
			key_state = pressed;

			auto code = pressed ? event_code::key_down : event_code::key_up;
			event_context context{};
			const int array_size{ 8 };
			context = std::array<int16_t, array_size>{ (int16_t)key };

			event::fire_event(code, nullptr, context);
		}
	}

	bool input::is_button_up(mouse_button button)
	{
		return !is_button_down(button);
	}

	bool input::is_button_down(mouse_button button)
	{
		return state.current_mouse.buttons[(int16_t)button];
	}

	bool input::was_button_up(mouse_button button)
	{
		return !was_button_down(button);
	}

	bool input::was_button_down(mouse_button button)
	{
		return state.previous_mouse.buttons[(int16_t)button];
	}

	void input::process_button(mouse_button button, bool pressed)
	{
		auto& button_state = state.current_mouse.buttons[(int16_t)button];

		if (button_state != pressed)
		{
			button_state = pressed;

			auto code = pressed ? event_code::mouse_down : event_code::mouse_up;
			event_context context{};
			const int array_size{ 8 };
			context = std::array<int16_t, array_size>{ (int16_t)button };
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