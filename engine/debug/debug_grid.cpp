#include <debug/debug_grid.h>

namespace egkr::debug
{
	debug_grid::shared_ptr debug_grid::create(const renderer_backend* backend, const configuration& configuration)
	{
		return std::make_shared<debug_grid>(backend, configuration);
	}

	debug_grid::debug_grid(const renderer_backend* backend, const configuration& configuration)
		: backend_{ backend },
		name_{ configuration.name },
		orientation_{ configuration.orientation },
		tile_count_dim0_{ configuration.tile_count_dim0 },
		tile_count_dim1_{ configuration.tile_count_dim1 },
		tile_scale_{ configuration.tile_scale }
	{
		auto max0 = tile_count_dim0_ * tile_scale_;
		auto min0 = -max0;

		auto max1 = tile_count_dim1_ * tile_scale_;
		auto min1 = -max1;

		switch (orientation_)
		{
		case egkr::debug::orientation::xy:
			extents_.min.x = min0;
			extents_.max.x = max0;
			extents_.min.y = min1;
			extents_.max.y = max1;
			break;
		case egkr::debug::orientation::yz:
			extents_.min.y = min0;
			extents_.max.y = max0;
			extents_.min.z = min1;
			extents_.max.z = max1;
			break;
		case egkr::debug::orientation::zx:
			extents_.min.z = min0;
			extents_.max.z = max0;
			extents_.min.x = min1;
			extents_.max.x = max1;
			break;
		default:
			break;
		}

		uint32_t vertex_count = ((tile_count_dim0_ * 2 + 1) * 2) + ((tile_count_dim1_ * 2 + 1) * 2);
		vertices_.resize(vertex_count);

		int32_t line_lenght0 = tile_count_dim1_ * tile_scale_;
		int32_t line_lenght1 = tile_count_dim0_ * tile_scale_;
		uint32_t element_index0{};
		uint32_t element_index1{};

		switch (orientation_)
		{
		case egkr::debug::orientation::xy:
			element_index0 = 0;
			element_index1 = 1;
			break;
		case egkr::debug::orientation::yz:
			element_index0 = 1;
			element_index1 = 2;
			break;
		case egkr::debug::orientation::zx:
			element_index0 = 2;
			element_index1 = 0;
			break;
		default:
			break;
		}

		vertices_[0].position[element_index0] = -line_lenght1;
		vertices_[0].position[element_index1] = 0;
		vertices_[0].colour[element_index0] = 1.F;
		vertices_[1].position[element_index0] = line_lenght1;
		vertices_[1].position[element_index1] = 0;
		vertices_[1].colour[element_index0] = 1.F;
		vertices_[2].position[element_index0] = 0;
		vertices_[2].position[element_index1] = line_lenght0;
		vertices_[2].colour[element_index1] = 1.F;
		vertices_[3].position[element_index0] = 0;
		vertices_[3].position[element_index1] = -line_lenght0;
		vertices_[3].colour[element_index1] = 1.F;

		float4 alt_line_colour = { 1.F, 1.F, 1.F, 0.5F };
		int32_t j = 1;
		for (uint32_t i{ 4 }; i < vertex_count; i += 8)
		{
			vertices_[i + 0].position[element_index0] = j * tile_scale_;
			vertices_[i + 0].position[element_index1] = line_lenght0;
			vertices_[i + 0].colour = alt_line_colour;

			vertices_[i + 1].position[element_index0] = j * tile_scale_;
			vertices_[i + 1].position[element_index1] = -line_lenght0;
			vertices_[i + 1].colour = alt_line_colour;

			vertices_[i + 2].position[element_index0] = -j * tile_scale_;
			vertices_[i + 2].position[element_index1] = line_lenght0;
			vertices_[i + 2].colour = alt_line_colour;

			vertices_[i + 3].position[element_index0] = -j * tile_scale_;
			vertices_[i + 3].position[element_index1] = -line_lenght0;
			vertices_[i + 3].colour = alt_line_colour;

			vertices_[i + 4].position[element_index0] = -line_lenght1;
			vertices_[i + 4].position[element_index1] = -j * tile_scale_;
			vertices_[i + 4].colour = alt_line_colour;

			vertices_[i + 5].position[element_index0] = line_lenght1;
			vertices_[i + 5].position[element_index1] = -j * tile_scale_;
			vertices_[i + 5].colour = alt_line_colour;

			vertices_[i + 6].position[element_index0] = -line_lenght1;
			vertices_[i + 6].position[element_index1] = j * tile_scale_;
			vertices_[i + 6].colour = alt_line_colour;

			vertices_[i + 7].position[element_index0] = line_lenght1;
			vertices_[i + 7].position[element_index1] = j * tile_scale_;
			vertices_[i + 7].colour = alt_line_colour;
			
			j++;
		}


	}

	debug_grid::~debug_grid()
	{
	}
	bool debug_grid::load()
	{
		geometry::properties properties{};
		properties.name = "degug_grid";
		properties.vertex_count = vertices_.size();
		properties.vertex_size = sizeof(colour_vertex_3d);
		properties.vertices = vertices_.data();

		geometry_ = geometry::geometry::create(backend_, properties);
		 
		geometry_->increment_generation();
		return true;
	}
	bool debug_grid::unload()
	{
		geometry_->destroy();
		geometry_.reset();
		return true;
	}
}