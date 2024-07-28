#include "ray.h"

namespace egkr
{
	ray ray::create(const float3& origin, const float3& direction)
	{
		return { .origin = origin, .direction = direction };
	}

	ray ray::from_screen(const int2& screen_position, const int2& viewport_size, const float3& origin, const float4x4& view, const float4x4& projection)
	{
		ray ray{};
		ray.origin = origin;

		float2 ndc{};
		ndc.x = 2.f * (float)screen_position.x / (float)viewport_size.x - 1;
		ndc.y = 1.f - 2.f * (float)screen_position.y / (float)viewport_size.y;

		const float4 clip{ ndc.x, ndc.y, -1.f, 1.f };
		float4 eye = glm::inverse(projection) * clip;

		eye.z = -1.f;
		eye.w = 0.f;

		ray.direction = glm::normalize(glm::inverse(view) * eye);
		return ray;
	}

	std::optional<float3> ray::aabb(const extent3d& extents) const
	{
		bool inside = true;
		std::array<int8_t, 3> quadrant{};
		float3 max_t{};
		float3 candidate_plane{};

		for (int32_t i{ 0 }; i < 3; ++i)
		{
			if (origin[i] < extents.min[i])
			{
				quadrant[(uint32_t)i] = 1;
				candidate_plane[i] = extents.min[i];
				inside = false;
			}
			else if (origin[i] > extents.max[i])
			{
				quadrant[(uint32_t)i] = 0;
				candidate_plane[i] = extents.max[i];
				inside = false;
			}
			else
			{
				quadrant[(uint32_t)i] = 2;
			}
		}

		if (inside)
		{
			return origin;
		}

		for (int32_t i{ 0 }; i < 3; ++i)
		{
			if (quadrant[(uint32_t)i] != 2 && direction[i] != 0.f)
			{
				max_t[i] = (candidate_plane[i] - origin[i]) / direction[i];
			}
			else
			{
				max_t[i] = -1.f;
			}
		}

		int32_t which_plane = 0;
		for (int32_t i{ 1 }; i < 3; ++i)
		{
			if (max_t[which_plane] < max_t[i])
			{
				which_plane = i;
			}
		}

		if (max_t[which_plane] < 0.f)
		{
			return {};
		}

		float3 out_point{};
		for (int32_t i{ 0 }; i < 3; ++i)
		{
			if (which_plane != i)
			{
				out_point[i] = origin[i] + max_t[which_plane] * direction[i];
				if (out_point[i] < extents.min[i] || out_point[i] > extents.max[i])
				{
					return {};
				}
			}
			else
			{
				out_point[i] = candidate_plane[i];
			}
		}
		return out_point;
	}

	std::optional<float> ray::oriented_extents(const extent3d& bb, const float4x4& model) const
	{
		auto inv = glm::inverse(model);
		ray ray{ .origin = inv * glm::vec4(origin, 1.f), .direction = inv * glm::vec4(direction, 0.f)};

		auto result =
			ray.aabb(bb)
			.transform([&](float3 out)
					   {
						   float3 out_point = model * glm::vec4(out, 1.f);
						   return glm::distance(origin, out_point);
					   });
		return result;
	}
}
