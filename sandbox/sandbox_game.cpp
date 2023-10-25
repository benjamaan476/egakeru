#include "sandbox_game.h"
#include "application/application.h"

#include "input.h"

sandbox_game::sandbox_game(const egkr::application_configuration& configuration)
	: game(configuration)
{
}

bool sandbox_game::init()
{
	LOG_INFO("Sandbox game created");

	camera_ = egkr::camera_system::get_default();
	return true;
}

void sandbox_game::update(double delta_time)
{
	auto temp_speed{ 50.F };

	if (egkr::input::is_key_down(egkr::key::a))
	{
		camera_->camera_yaw(1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::d))
	{
		camera_->camera_yaw(-1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::e))
	{
		camera_->camera_pitch(-1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::q))
	{
		camera_->camera_pitch(1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::w))
	{
		camera_->move_forward(temp_speed * (float)delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::x))
	{
		camera_->move_up(temp_speed * (float)delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::space))
	{
		camera_->move_down(temp_speed * (float)delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::s))
	{
		camera_->move_back(50.F * (float)delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::t))
	{
		egkr::event::fire_event(egkr::event_code::debug01, nullptr, {});
	}

	if (egkr::input::is_key_down(egkr::key::key_0))
	{
		const uint32_t array_size{ 4 };
		egkr::event_context context = std::array<uint32_t, array_size>{ 0U };
		egkr::event::fire_event(egkr::event_code::render_mode, nullptr, context);
	}
	if (egkr::input::is_key_down(egkr::key::key_1))
	{
		const uint32_t array_size{ 4 };
		egkr::event_context context = std::array<uint32_t, array_size>{ 1U };
		egkr::event::fire_event(egkr::event_code::render_mode, nullptr, context);
	}
	if (egkr::input::is_key_down(egkr::key::key_2))
	{
		const uint32_t array_size{ 4 };
		egkr::event_context context = std::array<uint32_t, array_size>{ 2U };
		egkr::event::fire_event(egkr::event_code::render_mode, nullptr, context);
	}
}

void sandbox_game::render(double /*delta_time*/)
{
}

void sandbox_game::resize(uint32_t /*width*/, uint32_t /*height*/) {}
