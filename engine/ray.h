#pragma once

#include "pch.h"

namespace egkr
{
	class ray
	{
	public:
		float3 origin;
		float3 direction;

		enum class hit_type
		{
			bounding_box,
			surface
		};

		struct hit
		{
			hit_type type;
			uint32_t unique_id;
			float3 position;
			float distance;
		};

		struct hit_result
		{
			egkr::vector <hit> hits;
			operator bool() const { return !hits.empty(); }
		};

		static ray create(const float3& origin, const float3& direction);
		static ray from_screen(const int2& screen_position, const float4& viewport_rect, const float3& origin, const float4x4& view, const float4x4& projection);

		[[nodiscard]] std::optional<float3> aabb(const extent3d& extents) const;
		[[nodiscard]] std::optional<float> oriented_extents(const extent3d& bb, const float4x4& model) const;
		[[nodiscard]] std::optional<std::tuple<float3, float>> plane(const std::optional<egkr::plane>& plane) const;
		[[nodiscard]] std::optional<std::tuple<float3, float>> disk(const std::optional<egkr::plane>& plane, const float3& center, float min_radius, float max_radius) const;
	};
}
