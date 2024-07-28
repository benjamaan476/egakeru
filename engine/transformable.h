#pragma once

#include "resources/transform.h"

namespace egkr
{
	struct transformable
	{
		void set_transform(const transform& transform) { transform_ = transform; }
		[[nodiscard]] const auto& get_transform() const { return transform_; }
		[[nodiscard]] auto& get_transform() { return transform_; }

		[[nodiscard]] auto get_world_transform() const
		{
			transform xform = transform_;
			return xform.get_world(); 
		}

		void set_position(const float3& position)
		{
			transform_.set_position(position);
		}
		[[nodiscard]] const auto& get_position() const { return transform_.get_position(); }

		void set_parent(transformable* parent)
		{
			transform_.set_parent(parent ? &parent->transform_ : nullptr);
		}

	protected:
		transform transform_{};
	};
}
