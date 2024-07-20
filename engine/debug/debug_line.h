#pragma once

#include <pch.h>
#include <resources/transform.h>
#include <resources/geometry.h>

namespace egkr::debug
{
	class debug_line
	{
	public:
		using shared_ptr = std::shared_ptr<debug_line>;
		static shared_ptr create(const float3& point_0, const float3& point_1);

		debug_line(const float3& point_0, const float3& point_1);

		void set_points(const float3& point_0, const float3& point_1);

	private:
		void recalculate_points();

	private:
		float3 point_0_{};
		float3 point_1_{};
		transform transform_{};
		egkr::vector<colour_vertex_3d> vertices_{};
		egkr::geometry::geometry::shared_ptr geometry_{};
	};
}

