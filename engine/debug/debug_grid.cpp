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
			break;
		case egkr::debug::orientation::yz:
			break;
		case egkr::debug::orientation::zx:
			break;
		default:
			break;
		}
	}

	debug_grid::~debug_grid()
	{
	}
}