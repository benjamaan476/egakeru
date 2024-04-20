#pragma once
#include "pch.h"

namespace egkr
{
	struct plane
	{
		egkr::float3 normal{};
		float distance{};

		constexpr static plane create(const float3& position, const float3& normal)
		{
			const auto norm = glm::normalize(normal);
			return { norm, glm::dot(norm, position) };
		}

		constexpr float signed_distance(const float3& position) const
		{
			return glm::dot(normal, position) - distance;
		}

		constexpr bool intersects_sphere(const float3& center, float radius) const
		{
			return signed_distance(center) > -radius;
		}

		constexpr bool intersects_aabb(const float3& center, const float3& half_extents) const
		{
			float r = half_extents.x * std::abs(normal.x) +
				half_extents.y * std::abs(normal.y) +
				half_extents.z * std::abs(normal.z);

			return -r <= signed_distance(center);
		}
	};
}