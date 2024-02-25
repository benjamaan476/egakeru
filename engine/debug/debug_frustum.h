#pragma once

#include <pch.h>
#include <resources/transform.h>
#include <resources/geometry.h>

#include <debug/debug_line.h>

namespace egkr::debug
{
	class debug_frustum
	{
	public:
		using shared_ptr = std::shared_ptr<debug_frustum>;
		static shared_ptr create(const egkr::frustum& frustum);
		explicit debug_frustum(const egkr::frustum& frustum);

		void update(const frustum& frustum);
		[[nodiscard]] const auto& get_geometry() const { return geometry_; }
		[[nodiscard]] const auto& get_transform() const { return transform_; }

		void destroy();
	private:
		void recalculate_lines(const frustum& frustum);

	private:
		egkr::transform transform_{};
		egkr::vector<colour_vertex_3d> vertices_{};
		egkr::geometry::geometry::shared_ptr geometry_{};
	};
}
