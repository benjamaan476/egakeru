#pragma once
#include <pch.h>
#include <renderer/vertex_types.h>

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
			uint32_t tile_count_dim0{ 1 };
			uint32_t tile_count_dim1{ 1 };
			float tile_scale{ 1.F };
			egkr::vector<colour_vertex_3d> vertices{};
		};

		class debug_grid
		{
		public:
			using shared_ptr = std::shared_ptr<debug_grid>;
			static shared_ptr create(const renderer_backend* backend, const configuration& configuration);

			debug_grid(const renderer_backend* backend, const configuration& configuration);
			~debug_grid();

		private:
			const renderer_backend* backend_{};
			uint32_t unique_id_{};
			std::string name_{};
			orientation orientation_{};
			uint32_t tile_count_dim0_{};
			uint32_t tile_count_dim1_{};
			float tile_scale_{};
			egkr::vector<colour_vertex_3d> vertices_{};
			extent3d extents_{};
		};
	}
}