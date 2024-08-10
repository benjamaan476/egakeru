#pragma once

#include <pch.h>
#include <interfaces/transformable.h>
#include <interfaces/renderable.h>

namespace egkr::debug
{
	class debug_line : public transformable, public renderable
	{
	public:
		using shared_ptr = std::shared_ptr<debug_line>;
		static shared_ptr create(const float3& point_0, const float3& point_1);

		debug_line(const float3& point_0, const float3& point_1);

		void set_points(const float3& point_0, const float3& point_1);

		void set_colour(uint32_t v, const float4& colour);
		void set_colour(const float4& colour);

	private:
		void recalculate_points();

	private:
		float3 point_0_{};
		float3 point_1_{};
		egkr::vector<colour_vertex_3d> vertices_{};
	};
}

