#include "transform.h"
#include <glm/gtx/quaternion.hpp>
namespace egkr
{
	transform transform::create()
	{
		return { float3{0.F}, glm::quat{{0, 0, 0}}, float3{1.F} };
	}
	transform transform::create(const float3& position)
	{
		return { position, glm::quat{{0, 0, 0}}, float3{ 1.F } };
	}
	transform transform::create(const glm::quat& rotation)
	{
		return { float3{ 0.F }, rotation, float3{ 1.F } };
	}
	transform transform::create(const float3& position, const glm::quat& rotation)
	{
		return { position, rotation, float3{ 1.F } };
	}
	transform transform::create(const float3& position, const glm::quat& rotation, const float3& scale)
	{
		return { position, rotation, scale };
	}

	transform::transform(const float3& position, const glm::quat& rotation, const float3& scale) : position_{ position }, rotation_{ rotation }, scale_{ scale } {}

	void transform::set_position(const float3& position)
	{
		position_ = position;
		is_dirty_ = true;
	}

	void transform::translate(const float3& position)
	{
		position_ += position;
		is_dirty_ = true;
	}

	void transform::set_rotation(const glm::quat& rotation)
	{
		rotation_ = rotation;
		is_dirty_ = true;
	}

	void transform::rotate(const glm::quat& rotation)
	{
		rotation_ *= rotation;
		is_dirty_ = true;
	}

	void transform::set_scale(const float3& scale)
	{
		scale_ = scale;
		is_dirty_ = true;
	}

	void transform::scale(const float3& scale)
	{
		scale_ *= scale;
		is_dirty_ = true;
	}

	void transform::set_parent(transform* parent)
	{
		parent_ = parent;
	}

	float4x4 transform::get_local()
	{
		if (is_dirty_)
		{
			float4x4 local{ 1.F };
			auto trans = glm::translate(local, position_);
			auto rot = glm::toMat4(rotation_);
			auto scale = glm::scale(local, scale_);
			local_ = trans * rot * scale;
			is_dirty_ = false;
		}

		return local_;
	}

	float4x4 transform::get_world()
	{
		auto local = get_local();

		if (parent_)
		{
			auto parent_world = parent_->get_world();
			local = parent_world * local;
		}

		return local;
	}
}