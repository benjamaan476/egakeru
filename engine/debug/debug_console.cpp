#include "debug_console.h"
#include "systems/console_system.h"
#include "systems/input.h"

namespace egkr
{
	static debug_console::unique_ptr state;

	debug_console* debug_console::create()
	{
		if (state)
		{
			LOG_ERROR("Debug console already initialised, can only have one instance");
			return nullptr;
		}

		state = std::make_unique<debug_console>();
		console::register_consumer(nullptr, &debug_console::consumer_write);
		return state.get();
	}

	bool debug_console::consumer_write(void* /*instance*/, log_level /*level*/, const std::string& message)
	{
		if (!state)
		{
			LOG_ERROR("Debug console not initialised before use");
			return false;
		}

		if (message.contains('\n'))
		{
			std::istringstream ss{ message };
			std::string line{};
			while (std::getline(ss, line, '\n'))
			{
				state->lines_.push_back(line);
			}
		}
		else
		{
			state->lines_.push_back(message);
		}

		state->is_dirty_ = true;
		return true;
	}

	bool debug_console::load()
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return false;
		}

		const auto font_size{ 32 };
		state->text_control_ = text::ui_text::create(text::type::bitmap, "Arial 32", font_size, "");
		state->text_control_->set_position({ 3, 30, 0 });

		state->entry_control_ = text::ui_text::create(text::type::bitmap, "Arial 32", font_size, "");
		state->entry_control_->set_position({ 3, 30 + (font_size * state->line_display_count_), 0 });

		event::register_event(event::code::key_down, nullptr, &on_key);

		return false;
	}

	void debug_console::update()
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return;
		}

		if (state->is_dirty_)
		{
			const int32_t line_count = state->lines_.size();
			auto max_lines = std::min(state->line_display_count_, std::max(line_count, state->line_display_count_));
			auto min_line = std::max(line_count - max_lines - state->line_offset_, 0);
			auto max_line = min_line + max_lines - 1;

			std::string buffer{};
			buffer.reserve(16384);

			for (auto i = min_line; i <= max_line; ++i)
			{
				if (i < line_count)
				{
					//TODO colour codes
					const auto& line = state->lines_[i];
					for (const char ch : line)
					{
						buffer.push_back(ch);
					}
				}
				buffer.push_back('\n');
			}
			state->text_control_->set_text(buffer);
			state->is_dirty_ = false;
		}
	}

	void debug_console::shutdown()
	{
		if (state)
		{
			state->text_control_.reset();
			state->entry_control_.reset();
			state.reset();
		}
	}

	bool debug_console::on_key(event::code code, void* /*sender*/, void* /*listener*/, const event::context& context)
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return false;
		}

		if (!state->is_visible_)
		{
			return false;
		}

		if (code == event::code::key_down)
		{
			uint16_t key_code{};
			context.get(0, key_code);
			const key key = (egkr::key)key_code;

			const bool shift_held = input::is_key_down(key::left_shift) || input::is_key_down(key::right_shift);

			if (key == key::enter)
			{
				if (state->entry_control_->has_text())
				{
					state->history_.emplace_back(state->entry_control_->get_text());
					console::execute_command(state->entry_control_->get_text());

					state->entry_control_->set_text("");
				}
			}
			else if (key == key::backspace)
			{
				if (state->entry_control_->has_text())
				{
					state->entry_control_->pop_back();
				}
			}
			else
			{
				char char_code = (char)key;
				if ((key >= key::a && key <= key::z))
				{
					if (!shift_held)
					{
						char_code += 32;
					}
					//TODO deal with caps lock
				}
				else if (key >= key::key_0 && key <= key::key_9)
				{
					if (shift_held)
					{
						switch (key)
						{

						case egkr::key::key_0:
							char_code = ')';
							break;
						case egkr::key::key_1:
							char_code = '!';
							break;
						case egkr::key::key_2:
							char_code = '"';
							break;
						case egkr::key::key_3:
							char_code = '#';
							break;
						case egkr::key::key_4:
							char_code = '$';
							break;
						case egkr::key::key_5:
							char_code = '%';
							break;
						case egkr::key::key_6:
							char_code = '^';
							break;
						case egkr::key::key_7:
							char_code = '&';
							break;
						case egkr::key::key_8:
							char_code = '*';
							break;
						case egkr::key::key_9:
							char_code = '(';
							break;
						default:
							LOG_ERROR("Shouldn't have got here");
							break;
						}
					}
				}
				else
				{
					switch (key)
					{
					case key::space:
						char_code = (char)key;
						break;
					case key::minus:
						char_code = shift_held ? '_' : '-';
						break;
					case key::equal:
						char_code = shift_held ? '+' : '=';
						break;
					default:
						char_code = 0;
					}
				}

				if (char_code != 0)
				{
					state->entry_control_->push_back(char_code);
				}
			}
			return false;
		}
		return false;
	}

	void debug_console::move_up()
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return;
		}

		state->is_dirty_ = true;
		const int32_t line_count = state->lines_.size();

		if (line_count <= state->line_display_count_)
		{
			state->line_offset_ = 0;
			return;
		}

		state->line_offset_++;
		state->line_offset_ = std::min(state->line_offset_, line_count - state->line_display_count_);
	}

	void debug_console::move_down()
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return;
		}
		state->is_dirty_ = true;
		const int32_t line_count = state->lines_.size();

		if (line_count <= state->line_display_count_)
		{
			state->line_offset_ = 0;
			return;
		}

		state->line_offset_--;
		state->line_offset_ = std::max(state->line_offset_, 0);
	}

	void debug_console::move_to_top()
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return;
		}

		state->is_dirty_ = true;
		const int32_t line_count = state->lines_.size();

		if (line_count <= state->line_display_count_)
		{
			state->line_offset_ = 0;
			return;
		}

		state->line_offset_ = line_count - state->line_display_count_;
	}

	void debug_console::move_to_bottom()
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return;
		}

		state->is_dirty_ = true;
		const int32_t line_count = state->lines_.size();

		if (line_count <= state->line_display_count_)
		{
			state->line_offset_ = 0;
			return;
		}

		state->line_offset_ = 0;

	}

	text::ui_text::shared_ptr debug_console::get_text()
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return nullptr;
		}

		return state->text_control_;
	}

	text::ui_text::shared_ptr debug_console::get_entry_text()
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return nullptr;
		}

		return state->entry_control_;
	}

	bool debug_console::is_visible()
	{
		if (!state)
		{
			LOG_ERROR("Debug console used before initialised");
			return false;
		}

		return state->is_visible_;
	}

	void debug_console::toggle_visibility()
	{
		state->is_visible_ = !state->is_visible_;
	}

}