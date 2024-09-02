#pragma once
#include "pch.h"
#include "keys.h"
#include "keymap.h"

#include "system.h"

namespace egkr
{
	enum class mouse_button : uint16_t
	{
		left,
		middle,
		right,

		button_count
	};

	struct keyboard_state
	{
		std::array<bool, (size_t)key::key_count> keys{};
	};
	
	struct mouse_state
	{
		int32_t x{};
		int32_t y{};
		std::array<bool, (size_t)mouse_button::button_count> buttons{};
		std::array<bool, (size_t)mouse_button::button_count> dragging{};
	};

	class input : public system
	{
	public:
		using unique_ptr = std::unique_ptr<input>;

		static system* create();

		bool init() override;
		bool update(const frame_data& frame_data) override;
		bool shutdown() override;

		~input() override = default;

		static bool is_key_down(key key);
		static bool is_key_up(key key);
		static bool was_key_up(key key);
		static bool was_key_down(key key);
		static bool was_key_released(key key);
		static bool was_key_pressed(key key);
		static void process_key(key key, bool pressed);

		static bool is_button_up(mouse_button button);
		static bool is_button_down(mouse_button button);
		static bool is_button_dragging(mouse_button button);
		static bool was_button_up(mouse_button button);
		static bool was_button_down(mouse_button button);
		static void process_button(mouse_button button, bool pressed);

		static int2 get_mouse_position();
		static int2 get_previous_mouse_position();
		static void process_mouse_move(int32_t xpos, int32_t ypos);
		static void process_mouse_wheel(double xoffset, double yoffset);

		static void push_keymap(const keymap& keymap);
		static void pop_keymap();

	private:
		static bool check_modifiers(keymap::modifier modifier);
		keyboard_state current_keyboard{};
		keyboard_state previous_keyboard{};

		mouse_state current_mouse{};
		mouse_state previous_mouse{};

		egkr::vector<keymap> keymaps{};
	};
}
