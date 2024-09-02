#include "camera.h"

namespace egkr
{
	camera::shared_ptr camera::create(std::string_view name)
	{
		return std::make_shared<camera>(name);
	}

	camera::camera(std::string_view name)
		:name_{ name }
	{
			float3 front = { glm::cos(rotation_.y) * glm::cos(rotation_.z), glm::cos(rotation_.y) * glm::sin(rotation_.z), glm::sin(rotation_.y)};
			view_ = glm::lookAt(position_, position_ - front, { 0.F, 0.F, 1.F });
	}

	void camera::reset()
	{
		position_ = {};
		rotation_ = {};
		is_dirty_ = false;
		view_ = {};
	}

	void camera::set_position(const float3& pos)
	{
		position_ = pos;
		is_dirty_ = true;
	}

	void camera::set_rotation(const float3& rot)
	{
		rotation_ = rot;
		is_dirty_ = true;
	}

	void camera::set_aspect(float aspect)
	{
		aspect_ratio_ = aspect;
	}

	float4x4 camera::get_view()
	{
		if (is_dirty_)
		{
			float3 front = { glm::cos(rotation_.y) * glm::cos(rotation_.z), glm::cos(rotation_.y) * glm::sin(rotation_.z), glm::sin(rotation_.y)};
			view_ = glm::lookAt(position_, position_ - front, { 0.F, 0.F, 1.F });
			is_dirty_ = false;
		}

		return view_;
	}

	float4x4 camera::get_projection() const
	{
		return glm::perspective(fov_, aspect_ratio_, near_clip_, far_clip_);
	}

	float3 camera::get_forward() const
	{
		return -glm::normalize(glm::inverse(view_)[2]);
	}

	float3 camera::get_back() const
	{
		return glm::normalize(glm::inverse(view_)[2]);
	}

	float3 camera::get_left() const
	{
		return -glm::normalize(glm::inverse(view_)[0]);
	}

	float3 camera::get_right() const
	{
		return glm::normalize(glm::inverse(view_)[0]);
	}

	float3 camera::get_up() const
	{
		return glm::normalize(glm::inverse(view_)[1]);
	}

	float3 camera::get_down() const
	{
		return -glm::normalize(glm::inverse(view_)[1]);
	}

	void camera::move_forward(float_t amount)
	{
		auto dir = get_forward();
		dir *= amount;
		position_ += dir;

		is_dirty_ = true;
	}

	void camera::move_back(float_t amount)
	{
		auto dir = get_back();
		dir *= amount;
		position_ += dir;

		is_dirty_ = true;
	}

	void camera::move_left(float_t amount)
	{
		auto dir = get_left();
		dir *= amount;
		position_ += dir;

		is_dirty_ = true;
	}

	void camera::move_right(float_t amount)
	{
		auto dir = get_right();
		dir *= amount;
		position_ += dir;

		is_dirty_ = true;
	}

	void camera::move_up(float_t amount)
	{
		float3 dir = { 0.F, 0.F, 1.F };
		dir *= amount;
		position_ += dir;

		is_dirty_ = true;
	}

	void camera::move_down(float_t amount)
	{
		float3 dir = { 0.F, 0.F, -1.F };
		dir *= amount;
		position_ += dir;

		is_dirty_ = true;
	}

	void camera::yaw(float amount)
	{
		rotation_.z += amount;
		is_dirty_ = true;
	}

	void camera::pitch(float amount)
	{
		rotation_.y += amount;
		is_dirty_ = true;
	}

	float camera::get_fov() const
	{
		return fov_;
	}

	float camera::get_near_clip() const
	{
		return near_clip_;
	}

	float camera::get_far_clip() const
	{
		return far_clip_;
	}
}
