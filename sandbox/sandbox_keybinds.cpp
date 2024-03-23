#include "sandbox_keybinds.h"
#include "pch.h"

#include "input.h"
#include "keymap.h"
#include "debug/debug_console.h"
#include "game/game.h"

using namespace egkr;
using egkr::key;
using egkr::keymap;

static void on_escape(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* /*user_data*/)
{
	LOG_INFO("Escape callback");
	event::fire_event(event_code::quit, nullptr, {});
}

static void on_yaw(key key, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	float f{};
	if (key == key::left || key == key::a)
	{
		f = 1.F;
	}
	else if (key == key::right || key == key::d)
	{
		f = -1.F;
	}

	game_instance->get_camera()->yaw(f * game_instance->get_delta_time());
}

static void on_pitch(key key, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	float f{};
	if (key == key::up)
	{
		f = 1.F;
	}
	else if (key == key::down)
	{
		f = -1.F;
	}

	game_instance->get_camera()->pitch(f * game_instance->get_delta_time());
}

static void on_move_forward(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	const float move_speed{ 50.f };
	game_instance->get_camera()->move_forward(move_speed * game_instance->get_delta_time());
}

static void on_move_backward(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	const float move_speed{ 50.f };
	game_instance->get_camera()->move_back(move_speed * game_instance->get_delta_time());
}

static void on_move_left(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	const float move_speed{ 50.f };
	game_instance->get_camera()->move_left(move_speed * game_instance->get_delta_time());
}

static void on_move_right(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	const float move_speed{ 50.f };
	game_instance->get_camera()->move_right(move_speed * game_instance->get_delta_time());
}

static void on_move_up(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	const float move_speed{ 50.f };
	game_instance->get_camera()->move_up(move_speed * game_instance->get_delta_time());
}

static void on_move_down(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	const float move_speed{ 50.f };
	game_instance->get_camera()->move_down(move_speed * game_instance->get_delta_time());
}

static void on_console_change_visibility(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	debug_console::toggle_visibility();

	if (debug_console::is_visible())
	{
		input::push_keymap(game_instance->get_console_keymap());
	}
	else
	{
		input::pop_keymap();
	}
}

static void on_set_render_mode_default(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;
	event_context context{};
	uint32_t mode{ 0 };
	context.set(0, mode);
	event::fire_event(event_code::render_mode, game_instance, context);
}

static void on_set_render_mode_lighting(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;
	event_context context{};
	uint32_t mode{ 1 };
	context.set(0, mode);
	event::fire_event(event_code::render_mode, game_instance, context);
}

static void on_set_render_mode_normals(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;
	event_context context{};
	uint32_t mode{ 2 };
	context.set(0, mode);
	event::fire_event(event_code::render_mode, game_instance, context);
}

static void on_load_scene(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* /*user_data*/)
{
	event::fire_event(event_code::debug02, nullptr, {});
}

static void on_console_scroll(key key, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* /*user_data*/)
{
	if (key == key::up)
	{
		debug_console::move_up();
	}
	else if (key == key::down)
	{
		debug_console::move_down();
	}
}

static void on_console_scroll_hold(key key, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* user_data)
{
	game* game_instance = (game*)user_data;

	static float accumulated_time{};
	accumulated_time += (float)game_instance->get_delta_time();

	if (accumulated_time > 0.1F)
	{
		if (key == key::up)
		{
			debug_console::move_up();
		}
		else if (key == key::down)
		{
			debug_console::move_down();
		}
		accumulated_time = 0.F;
	}
}

static void on_change_texture(key /*key*/, keymap::entry_bind_type /*type*/, keymap::modifier /*modifier*/, void* /*user_data*/)
{
	event::fire_event(event_code::debug01, nullptr, {});
}

void egkr::setup_keymaps(game* game_instance)
{
	keymap global_keymap = keymap::create();
	global_keymap.add_binding(key::esc, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_escape);

	input::push_keymap(global_keymap);

	keymap sandbox_keymap = keymap::create();
	sandbox_keymap.add_binding(key::a, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_yaw);
	sandbox_keymap.add_binding(key::left, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_yaw);
	sandbox_keymap.add_binding(key::d, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_yaw);
	sandbox_keymap.add_binding(key::right, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_yaw);

	sandbox_keymap.add_binding(key::up, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_pitch);
	sandbox_keymap.add_binding(key::down, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_pitch);

	sandbox_keymap.add_binding(key::grave, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_console_change_visibility);

	sandbox_keymap.add_binding(key::w, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_move_forward);
	sandbox_keymap.add_binding(key::s, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_move_backward);
	sandbox_keymap.add_binding(key::q, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_move_left);
	sandbox_keymap.add_binding(key::e, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_move_right);
	sandbox_keymap.add_binding(key::space, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_move_up);
	sandbox_keymap.add_binding(key::x, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_move_down);

	sandbox_keymap.add_binding(key::key_0, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_set_render_mode_default);
	sandbox_keymap.add_binding(key::key_1, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_set_render_mode_lighting);
	sandbox_keymap.add_binding(key::key_2, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_set_render_mode_normals);

	sandbox_keymap.add_binding(key::l, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_load_scene);
	sandbox_keymap.add_binding(key::t, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_change_texture);

	input::push_keymap(sandbox_keymap);

	game_instance->get_console_keymap() = keymap::create();
	auto& console_keymap = game_instance->get_console_keymap();
	console_keymap.overrides_all = true;

	console_keymap.add_binding(key::grave, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_console_change_visibility);
	console_keymap.add_binding(key::esc, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_console_change_visibility);

	console_keymap.add_binding(key::up, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_console_scroll);
	console_keymap.add_binding(key::up, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_console_scroll_hold);
	console_keymap.add_binding(key::down, keymap::entry_bind_type::press, keymap::modifier::none, game_instance, on_console_scroll);
	console_keymap.add_binding(key::down, keymap::entry_bind_type::hold, keymap::modifier::none, game_instance, on_console_scroll_hold);
}
