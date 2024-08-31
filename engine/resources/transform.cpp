#include "transform.h"
#include <glm/gtx/quaternion.hpp>
namespace egkr
{
	void transformable::set_position(const float3& position)
	{
		position_ = position;
		is_dirty_ = true;
	}

	void transformable::translate(const float3& position)
	{
		position_ += position;
		is_dirty_ = true;
	}

	void transformable::set_rotation(const glm::quat& rotation)
	{
		rotation_ = rotation;
		is_dirty_ = true;
	}

	void transformable::rotate(const glm::quat& rotation)
	{
		rotation_ *= rotation;
		is_dirty_ = true;
	}

	void transformable::set_scale(const float3& scale)
	{
		scale_ = scale;
		is_dirty_ = true;
	}

	void transformable::scale(const float3& scale)
	{
		scale_ *= scale;
		is_dirty_ = true;
	}

	void transformable::set_parent(const std::shared_ptr<transformable>& parent)
	{
		parent_ = parent;
	}

	float4x4 transformable::get_local()
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

	float4x4 transformable::get_world()
	{
		auto local = get_local();

		if (auto parent = parent_.lock())
		{
			auto parent_world = parent->get_world();
			local = parent_world * local;
		}

		return local;
	}
}
