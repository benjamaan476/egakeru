#include "debug_line.h"

namespace egkr::debug
{
	debug_line::shared_ptr debug_line::create(const float3& point_0, const float3& point_1)
	{
		return std::make_shared<debug_line>(point_0, point_1);
	}
	debug_line::debug_line(const float3& point_0, const float3& point_1)
		: point_0_{ point_0 }, point_1_{point_1}
	{
		vertices_.resize(2);
		recalculate_points();

		geometry::properties properties{
			.name = "debug_line",
			.vertex_size = sizeof(colour_vertex_3d),
			.vertex_count = (uint32_t)vertices_.size(),
			.vertices = vertices_.data()
		};

		geometry_ = geometry::geometry::create(properties);
		geometry_->increment_generation();
	}

	void debug_line::set_points(const float3& point_0, const float3& point_1)
	{
		point_0_ = point_0;
		point_1_ = point_1;

		recalculate_points();

		geometry_->update_vertices(0, (uint32_t)vertices_.size(), vertices_.data());

	}

	void debug_line::recalculate_points()
	{
		vertices_[0].position = { point_0_, 1.F };
		vertices_[1].position = { point_1_, 1.F };
	}
}
