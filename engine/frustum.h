#pragma once
#include "plane.h"

namespace egkr
{
	struct frustum
	{
		std::array<plane, 6> sides{};
		float3 position{};
		float3 forward{};
		float3 right{};
		float3 up{};
		float fov{};
		float near{};
		float far{};
		float aspect{};

		frustum() = default;
		constexpr frustum(const float3& position, const float3& forward, const float3& right, const float3& up, float aspect, float fov, float near, float far)
			: position{ position }, forward{ forward }, right{ right }, up{ up }, fov{ fov }, near{ near }, far{ far }, aspect{ aspect }
		{
			const float half_v = far * std::tanf(0.5f * fov);
			const float half_h = half_v * aspect;

			const auto forward_fwd = forward * far;
			const auto half_right = half_h * right;
			const auto half_up = half_v * up;

			sides[0] = plane::create(position + near * forward, forward);
			sides[1] = plane::create(position + forward_fwd, -forward);
			sides[2] = plane::create(position, glm::cross(up, half_right + forward_fwd));
			sides[3] = plane::create(position, glm::cross(forward_fwd - half_right, up));
			sides[4] = plane::create(position, glm::cross(right, forward_fwd - half_up));
			sides[5] = plane::create(position, glm::cross(half_up + forward_fwd, right));

		};

		bool intersects_sphere(const float3& center, float radius) const
		{
			for (const auto& side : sides)
			{
				if (!side.intersects_sphere(center, radius))
				{
					return false;
				}
			}
			return true;
			//return std::ranges::all_of(sides, [center, radius](const auto& plane) { return plane.intersects_sphere(center, radius); });
		}

		constexpr bool intersects_aabb(const float3& center, const float3& half_extents) const
		{
			return std::ranges::all_of(sides, [center, half_extents](const auto& plane) { return plane.intersects_aabb(center, half_extents); });
		}
	};
}