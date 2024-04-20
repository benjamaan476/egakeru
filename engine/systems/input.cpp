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
		keymaps.reserve(5);
		return true;
	}

	bool input::update(const frame_data& /*frame_data*/)
	{
		previous_keyboard = current_keyboard;
		previous_mouse = current_mouse;

		for (auto i{ 0U }; i < std::to_underlying(key::key_count); ++i)
		{
			if (is_key_down((key)i) && was_key_down((key)i))
			{
				for (auto& map : keymaps | std::views::reverse)
				{
					auto* binding = map.entries[i].bindings;
					bool unset{};

					while (binding)
					{
						if (binding->type == keymap::entry_bind_type::unset)
						{
							unset = true;
							break;
						}
						else if (binding->type == keymap::entry_bind_type::hold)
						{
							if (binding->callback && check_modifiers(binding->modifier))
							{
								binding->callback((key)i, binding->type, binding->modifier, binding->user_data);
							}
						}
						binding = binding->next;
					}

					if (unset || map.overrides_all)
					{
						break;
					}
				}
			}
		}

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

			for (auto i = state->keymaps.size() - 1; i > 0; --i)
			{
				//Need to do it this way as keys can remove keymaps which invalidates iterators
				auto& map = state->keymaps[i];
				auto* binding = map.entries[std::to_underlying(key)].bindings;
				bool unset{};

				while (binding)
				{
					if (binding->type == keymap::entry_bind_type::unset)
					{
						unset = true;
						break;
					}
					else if (pressed && binding->type == keymap::entry_bind_type::press)
					{
						if (binding->callback && check_modifiers(binding->modifier))
						{
							binding->callback(key, binding->type, binding->modifier, binding->user_data);
						}
					}
					else if (!pressed && binding->type == keymap::entry_bind_type::release)
					{
						if (binding->callback && check_modifiers(binding->modifier))
						{
							binding->callback(key, binding->type, binding->modifier, binding->user_data);
						}
					}

					binding = binding->next;
				}

				if (unset || map.overrides_all)
				{
					break;
				}
			}

			auto code = pressed ? event::code::key_down : event::code::key_up;
			event::context context{};
			context.set(0, std::to_underlying(key));

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

			auto code = pressed ? event::code::mouse_down : event::code::mouse_up;
			event::context context{};
			context.set(0, (int16_t)button);
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

	void input::push_keymap(const keymap& keymap)
	{
		state->keymaps.push_back(keymap);
	}

	void input::pop_keymap()
	{
		state->keymaps.pop_back();
	}

	bool input::check_modifiers(keymap::modifier modifier)
	{
		if (modifier & keymap::modifier::shift)
		{
			if (!is_key_down(key::left_shift) && !is_key_down(key::right_shift))
			{
				return false;
			}
		}

		if (modifier & keymap::modifier::control)
		{
			if (!is_key_down(key::left_control) && !is_key_down(key::right_control))
			{
				return false;
			}
		}

		if (modifier & keymap::modifier::alt)
		{
			if (!is_key_down(key::left_alt) && !is_key_down(key::right_alt))
			{
				return false;
			}
		}

		return true;
	}
}