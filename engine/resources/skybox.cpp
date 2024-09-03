#include "skybox.h"

#include <systems/texture_system.h>
#include "identifier.h"
#include <systems/geometry_system.h>
#include <systems/shader_system.h>

namespace egkr
{
	skybox::shared_ptr skybox::create(const configuration& configuration)
	{
		return std::make_shared<skybox>(configuration);
	}

	void skybox::destroy()
	{
		//if (instance_id_ != invalid_32_id)
		{
			unload();
		}

		geometry_.reset();
		cubemap_.reset();
	}

	skybox::skybox(const configuration& configuration)
		: resource(0, invalid_32_id, "skybox"), configuration_{configuration}
	{
		configuration_.texture_map_properties = egkr::texture_map::properties
		{
		.minify = texture_map::filter::linear,
		.magnify = texture_map::filter::linear,
		.repeat_u = texture_map::repeat::clamp_to_edge,
		.repeat_v = texture_map::repeat::clamp_to_edge,
		.repeat_w = texture_map::repeat::clamp_to_edge,
		.use = texture_map::use::map_cube,
		};

		configuration_.geometry_properties = egkr::geometry_system::generate_cube(10, 10, 10, 1, 1, configuration.name, "");
	}

	bool skybox::load()
	{
		cubemap_ = egkr::texture_map::texture_map::create(configuration_.texture_map_properties);
		cubemap_->acquire();
		cubemap_->texture = texture_system::acquire_cube(configuration_.name);

		geometry_ = egkr::geometry_system::acquire(configuration_.geometry_properties);

		auto skybox_shader = egkr::shader_system::get_shader("Shader.Builtin.Skybox");
		egkr::vector<egkr::texture_map::texture_map::shared_ptr> maps = { cubemap_ };
		skybox_shader->acquire_instance_resources(maps);

		return true;
	}

	bool skybox::unload()
	{
		cubemap_->release();
		cubemap_.reset();
		geometry_->destroy();

		return false;
	}

	void skybox::set_frame_number(uint64_t frame_number)
	{
		render_frame_number_ = frame_number;
	}

	void skybox::set_draw_index(uint64_t draw_index)
	{
		draw_index_ = draw_index;
	}
}
