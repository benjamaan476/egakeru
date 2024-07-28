#pragma once
#include <pch.h>
#include <renderer/vertex_types.h>
#include <resources/geometry.h>
#include <transformable.h>

namespace egkr
{
	class renderer_backend;
	namespace debug
	{
		enum class orientation
		{
			xy, yz, zx
		};

		struct configuration
		{
			std::string name{};
			orientation orientation{};
			float tile_count_dim0{ 1.F };
			float tile_count_dim1{ 1.F };
			float tile_scale{ 1.F };
			bool use_third_axis{};
			egkr::vector<colour_vertex_3d> vertices{};
		};

		class debug_grid : public transformable
		{
		public:
			using shared_ptr = std::shared_ptr<debug_grid>;
			static shared_ptr create(const configuration& configuration);

			explicit debug_grid(const configuration& configuration);
			~debug_grid();

			bool load();
			bool unload();

		[[nodiscard]] const auto& get_geometry() const { return geometry_; }
		[[nodiscard]] const auto& get_id() const { return unique_id_; }

		private:
			uint32_t unique_id_{invalid_32_id};
			std::string name_{};
			orientation orientation_{};
			float tile_count_dim0_{};
			float tile_count_dim1_{};
			float tile_scale_{};
			egkr::vector<colour_vertex_3d> vertices_{};
			extent3d extents_{};
			geometry::geometry::shared_ptr geometry_{};
		};
	}
}
