#pragma once
#include "pch.h"

namespace egkr
{
	class transformable
	{
	public:
		[[nodiscard]] const auto& get_position() const { return position_; }
		void set_position(const float3& position);
		void translate(const float3& position);

		[[nodiscard]] const auto& get_rotation() const { return rotation_; }
		void set_rotation(const glm::quat& rotation);
		void rotate(const glm::quat& rotation);

		[[nodiscard]] const auto& get_scale() const { return scale_; }
		void set_scale(const float3& scale);
		void scale(const float3& scale);

		[[nodiscard]] float get_determinant() const { return determinant_; }

		void set_parent(const std::shared_ptr<transformable>& parent);

		[[nodiscard]] float4x4 get_local();
		[[nodiscard]] float4x4 get_world();
	private:
		float3 position_{0.F};
		glm::quat rotation_{{ 0, 0, 0 }};
		float3 scale_{1.F};

		float determinant_{ 0.f };

		bool is_dirty_{ true };
		
		float4x4 local_{1.F};
		std::weak_ptr<transformable> parent_;
	};
}
