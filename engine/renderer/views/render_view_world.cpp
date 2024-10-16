#include "render_view_world.h"
#include <systems/resource_system.h>
#include <systems/shader_system.h>
#include <systems/material_system.h>
#include <renderer/renderer_types.h>
#include <renderer/renderer_frontend.h>

namespace egkr
{
	render_view_world::render_view_world(const configuration& configuration)
		: render_view(configuration)
	{
	}

	bool render_view_world::on_create()
	{
		const std::string shader_name = "Shader.Builtin.Material";
		auto resource = resource_system::load(shader_name, resource::type::shader, nullptr);
		auto shader = (shader::properties*)resource->data;
		shader_system::create_shader(*shader, renderpasses_[0].get());
		resource_system::unload(resource);

		shader_ = shader_system::get_shader(shader_name);

		projection_ = glm::perspective(camera_->get_fov(), (float)width_ / (float)height_, camera_->get_near_clip(), camera_->get_far_clip());
		ambient_colour_ = { 0.25F, 0.25F, 0.25F, 1.F };

		event::register_event(event::code::render_mode, this, on_event);

		std::string colour_3d_shader_name = "Shader.Builtin.Colour3DShader";
		auto colour_shader = resource_system::load(colour_3d_shader_name, egkr::resource::type::shader, nullptr);
		auto properties = (shader::properties*)colour_shader->data;
		shader_system::create_shader(*properties, renderpasses_[0].get());
		resource_system::unload(colour_shader);

		auto col_shader = shader_system::get_shader(colour_3d_shader_name);
		locations_.projection = col_shader->get_uniform_index("projection");
		locations_.view = col_shader->get_uniform_index("view");
		locations_.model = col_shader->get_uniform_index("model");
		event::register_event(event::code::render_target_refresh_required, this, on_event);
		return true;
	}
	bool render_view_world::on_destroy()
	{
		event::unregister_event(event::code::render_target_refresh_required, this, on_event);
		return true;
	}
	void render_view_world::on_resize(uint32_t width, uint32_t height)
	{
		if (width != width_ || height != height_)
		{
			width_ = width;
			height_ = height;
			projection_ = glm::perspective(camera_->get_fov(), (float)width_ / (float)height_, camera_->get_near_clip(), camera_->get_far_clip());

			for (auto& pass : renderpasses_)
			{
				pass->set_render_area(width_, height_);
			}
		}
	}
	render_view_packet render_view_world::on_build_packet(void* data)
	{
		frame_geometry_data* mesh_data = (frame_geometry_data*)data;

		render_view_packet packet{};

		packet.render_view = this;
		packet.projection_matrix = projection_;
		packet.view_matrix = camera_->get_view();
		packet.view_position = camera_->get_position();
		packet.ambient_colour = ambient_colour_;
		packet.render_data = mesh_data->world_geometries;
		packet.debug_render_data = mesh_data->debug_geometries;

		return packet;
	}
	bool render_view_world::on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const
	{
		for (auto& pass : renderpasses_)
		{
			pass->begin(pass->get_render_targets()[render_target_index].get());

			shader_system::use(shader_->get_id());

			material_system::apply_global(shader_->get_id(), frame_number, render_view_packet->projection_matrix, render_view_packet->view_matrix, render_view_packet->ambient_colour, render_view_packet->view_position, mode_);
			for (egkr::render_data render_data : render_view_packet->render_data)
			{
				auto m = render_data.geometry->get_material();

				bool needs_update = m->get_render_frame() != frame_number;

				material_system::apply_instance(m, needs_update);
				m->set_render_frame(frame_number);

				if (auto transform = render_data.transform.lock())
				{
					material_system::apply_local(m, transform->get_world());
				}

				if (render_data.is_winding_reversed)
				{
					renderer->set_winding(winding::clockwise);
				}
				render_data.geometry->draw();

				if (render_data.is_winding_reversed)
				{
					renderer->set_winding(winding::counter_clockwise);
				}


			}

			if (!render_view_packet->debug_render_data.empty())
			{
				auto colour_shader = shader_system::get_shader("Shader.Builtin.Colour3DShader");
				shader_system::use(colour_shader->get_id());
				shader_system::set_uniform(locations_.projection, &render_view_packet->projection_matrix);
				shader_system::set_uniform(locations_.view, &render_view_packet->view_matrix);
				shader_system::apply_global(true);
				for (render_data data : render_view_packet->debug_render_data)
				{
					if (auto transform = data.transform.lock())
					{
						const auto& model = transform->get_world();
						shader_system::set_uniform(locations_.model, &model);
					}
					data.geometry->draw();
				}
				colour_shader->set_frame_number(frame_number);
			}
			pass->end();
		}
		return true;
	}
}
