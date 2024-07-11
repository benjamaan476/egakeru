#pragma once

#include "pch.h"
#include "resources/geometry.h"

#include <systems/system.h>

namespace egkr
{
	class geometry_system : public system
	{
	public:
		using geometry_reference = uint32_t;
		using unique_ptr = std::unique_ptr<geometry_system>;

		static geometry_system* create();
		geometry_system();
		~geometry_system() override;

		bool init() override;
		bool shutdown() override;

		static geometry::geometry::shared_ptr acquire(uint32_t id);
		static geometry::geometry::shared_ptr acquire(const geometry::properties& properties);

		static void release_geometry(const geometry::geometry::shared_ptr& geometry);

		static geometry::geometry::shared_ptr get_default();
		static geometry::properties generate_cube(float width, float height, float depth, uint32_t tile_x, uint32_t tile_y, std::string_view name, std::string_view material_name);
	private:
		static geometry::properties generate_plane(uint32_t width, uint32_t height, uint32_t x_segments, uint32_t y_segments, uint32_t tile_x, uint32_t tile_y, std::string_view name, std::string_view material_name);

	private:
		uint32_t max_geometry_count_{};

		geometry::geometry::shared_ptr default_geometry_{};
		egkr::vector<geometry::geometry::shared_ptr> registered_geometries_{};


	};
}
