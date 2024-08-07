#pragma once
#include "pch.h"

namespace egkr
{
	class transform
	{
	public:
		static transform create();
		static transform create(const float3& position);
		static transform create(const glm::quat& rotation);
		static transform create(const float3& position, const glm::quat& rotation);
		static transform create(const float3& position, const glm::quat& rotation, const float3& scale);

		transform() = default;
		transform(const float3& position, const glm::quat& rotation, const float3& scale);

		[[nodiscard]] const auto& get_position() const { return position_; }
		void set_position(const float3& position);
		void translate(const float3& position);

		[[nodiscard]] const auto& get_rotation() const { return rotation_; }
		void set_rotation(const glm::quat& rotation);
		void rotate(const glm::quat& rotation);

		[[nodiscard]] const auto& get_scale() const { return scale_; }
		void set_scale(const float3& scale);
		void scale(const float3& scale);

		void set_parent(transform* parent);

		[[nodiscard]] float4x4 get_local();
		[[nodiscard]] float4x4 get_world();
	private:
		float3 position_{0.F};
		glm::quat rotation_{{ 0, 0, 0 }};
		float3 scale_{1.F};

		bool is_dirty_{ true };
		
		float4x4 local_{1.F};
		transform* parent_{nullptr};
	};
}
