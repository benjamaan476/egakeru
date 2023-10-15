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

	recalculate_view_matrix();
	return true;
}

void sandbox_game::update(double delta_time)
{
	auto* app = get_application();

	if (egkr::input::is_key_down(egkr::key::a))
	{
		camera_yaw(1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::d))
	{
		camera_yaw(-1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::e))
	{
		camera_pitch(-1.F * delta_time);
	}

	if (egkr::input::is_key_down(egkr::key::q))
	{
		camera_pitch(1.F * delta_time);
	}

	egkr::float3 velocity{0};

	if (egkr::input::is_key_down(egkr::key::w))
	{
		egkr::float3 front{};
		front.x = view_[0][2];
		front.y = view_[1][2];
		front.z = view_[2][2];
		velocity -= front;
		view_dirty = true;
	}

	if (egkr::input::is_key_down(egkr::key::s))
	{
		egkr::float3 front{};
		front.x = view_[0][2];
		front.y = view_[1][2];
		front.z = view_[2][2];
		velocity += front;
		view_dirty = true;

	}

	if (egkr::input::is_key_down(egkr::key::t))
	{
		egkr::event::fire_event(egkr::event_code::debug01, nullptr, {});
	}

	position_ += velocity * 50.F * (float)delta_time;

	recalculate_view_matrix();

	//TODO Hack
	app->get_state().renderer->set_view(view_, position_);
}

void sandbox_game::render(double /*delta_time*/)
{
}

void sandbox_game::resize(uint32_t /*width*/, uint32_t /*height*/) {}

void sandbox_game::recalculate_view_matrix()
{
	if (view_dirty)
	{
		//egkr::float4x4 view{ 1.F };
		front = { glm::cos(rotation_.y) * glm::cos(rotation_.z), glm::cos(rotation_.y) * glm::sin(rotation_.z), glm::sin(rotation_.y)};
		view_ = glm::lookAt(position_, position_ - front, { 0.F, 0.F, 1.F });
		//view = glm::translate(view, position_);

		//egkr::float3 yaw{0, 0, 1};
		//view = glm::rotate(view, rotation_.z, yaw);
		//egkr::float3 pitch{ 0, 1, 0 };
		//view = glm::rotate(view, rotation_.y, pitch);
		//egkr::float3 roll{ 1, 0, 0 };
		//view = glm::rotate(view, rotation_.x, roll);


		//view_ = glm::inverse(view);
		view_dirty = false;
	}
}

void sandbox_game::camera_yaw(float amount)
{
	rotation_.z += amount;
	view_dirty = true;
}

void sandbox_game::camera_pitch(float amount)
{
	rotation_.y += amount;
	view_dirty = true;
}
