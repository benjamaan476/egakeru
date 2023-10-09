#pragma once

#include "pch.h"
#include "resources/geometry.h"

namespace egkr
{

	class geometry_system
	{
	public:
		using geometry_reference = uint32_t;
		using unique_ptr = std::unique_ptr<geometry_system>;

		static bool create(const void* renderer_context);
		geometry_system(const void* renderer_context);
		~geometry_system();

		static bool init();
		static void shutdown();

		static geometry::shared_ptr acquire(uint32_t id);
		static geometry::shared_ptr acquire(const geometry_properties& properties);

		static geometry::shared_ptr get_default();
	private:
		constexpr static geometry_properties generate_plane(uint32_t width, uint32_t height, uint32_t x_segments, uint32_t y_segments, uint32_t tile_x, uint32_t tile_y, std::string_view name, std::string_view material_name);

	private:
		const void* renderer_context_{};
		uint32_t max_geometry_count_{};

		geometry::shared_ptr default_geometry_{};
		egkr::vector<geometry::shared_ptr> registered_geometries_{};


	};
}
