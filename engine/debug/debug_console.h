#pragma once
#include "pch.h"
#include "event.h"

#include <resources/ui_text.h>

namespace egkr
{

	class debug_console
	{
	public:
		using unique_ptr = std::unique_ptr<debug_console>;
		static debug_console* create();

		static bool consumer_write(void* instance, log_level level, const std::string& message);

		static bool load();
		static void update();
		static void shutdown();

		static bool on_key(event::code code, void* sender, void* listener, const event::context& context);

		static void move_up();
		static void move_down();
		static void move_to_top();
		static void move_to_bottom();

		static text::ui_text::shared_ptr get_text();
		static text::ui_text::shared_ptr get_entry_text();

		static bool is_visible();
		static void toggle_visibility();


	private:
		int32_t line_display_count_{ 10 };
		int32_t line_offset_{};
		egkr::vector<std::string> lines_{};

		text::ui_text::shared_ptr text_control_{};
		text::ui_text::shared_ptr entry_control_{};

		struct command_history_entry
		{
			std::string command{};
		};

		egkr::vector<command_history_entry> history_{};
		//int32_t history_offset_{};
		bool is_dirty_{};
		bool is_visible_{false};
	};
}