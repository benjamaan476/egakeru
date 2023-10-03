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

	egkr::float4x4 view{1};
	view = glm::translate(view, { 0.F, 0.F, 30.F });
	view_ = glm::inverse(view);
	return true;
}

void sandbox_game::update(std::chrono::milliseconds /*delta_time*/)
{
	auto* app = get_application();

	if (egkr::input::is_key_down(egkr::key::a))
	{
		camera_yaw(0.001F);
	}

	if (egkr::input::is_key_down(egkr::key::d))
	{
		camera_yaw(-0.001F);
	}

	egkr::float3 velocity{0};

	if (egkr::input::is_key_down(egkr::key::w))
	{
		egkr::float3 front{};
		front.x = view_[0][2];
		front.y = view_[1][2];
		front.z = view_[2][2];
		velocity -= front;
	}

	if (egkr::input::is_key_down(egkr::key::s))
	{
		egkr::float3 front{};
		front.x = view_[0][2];
		front.y = view_[1][2];
		front.z = view_[2][2];
		velocity += front;

	}

	if (egkr::input::is_key_down(egkr::key::t))
	{
		egkr::event::fire_event(egkr::event_code::debug01, nullptr, {});
	}

	position_ += velocity * 0.01f;
	view_dirty = true;

	recalculate_view_matrix();

	//TODO Hack
	app->get_state().renderer->set_view(view_);
}

void sandbox_game::render(std::chrono::milliseconds /*delta_time*/)
{
}

void sandbox_game::resize(uint32_t /*width*/, uint32_t /*height*/) {}

void sandbox_game::recalculate_view_matrix()
{
	if (view_dirty)
	{
		egkr::float4x4 view{ 1 };
		view = glm::translate(view, position_);
		egkr::float3 up{ 0, 1, 0 };
		view = glm::rotate(view, rotation_.y, up);

		egkr::float3 pitch{ 1, 0, 0 };
		view = glm::rotate(view, rotation_.x, pitch);


		view_ = glm::inverse(view);
	}
	view_dirty = false;
}

void sandbox_game::camera_yaw(float amount)
{
	rotation_.y += amount;
	view_dirty = true;
}
