#include "skybox.h"

#include <systems/texture_system.h>

namespace egkr::skybox
{
	skybox::shared_ptr skybox::create(const renderer_backend* backend)
	{
		return std::make_shared<skybox>(backend);
	}

	void skybox::destroy()
	{
		geometry_->destroy();
		geometry_.reset();
		cubemap_->free();
		cubemap_.reset();
	}

	skybox::skybox(const renderer_backend* backend)
		: resource(0, invalid_32_id, "skybox")
	{
		egkr::texture_map::properties properties
		{
		.minify = texture_map::filter::linear,
		.magnify = texture_map::filter::linear,
		.repeat_u = texture_map::repeat::clamp_to_edge,
		.repeat_v = texture_map::repeat::clamp_to_edge,
		.repeat_w = texture_map::repeat::clamp_to_edge,
		.use = texture_map::use::map_cube,
		};

		cubemap_ = egkr::texture_map::texture_map::create(backend, properties);
		cubemap_->acquire();
		cubemap_->texture = texture_system::acquire_cube("skybox");


	}
	void skybox::set_frame_number(uint64_t frame_number)
	{
		render_frame_number_ = frame_number;
	}
	void skybox::set_geometry(geometry::geometry::shared_ptr geo)
	{
		geometry_ = geo;
	}
}