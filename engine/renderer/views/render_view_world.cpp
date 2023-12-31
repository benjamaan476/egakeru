#include "render_view_world.h"
#include <systems/resource_system.h>
#include <systems/shader_system.h>
#include <systems/material_system.h>
#include <systems/camera_system.h>
#include <renderer/renderer_types.h>

namespace egkr::render_view
{


	render_view_world::render_view_world(const renderer_frontend* renderer, const configuration& configuration)
		: render_view(renderer, configuration)
	{}
	bool render_view_world::on_create()
	{
		auto shader = shader_system::get_shader(custom_shader_name_ != "" ? custom_shader_name_ : BUILTIN_SHADER_NAME_MATERIAL);
		shader_id_ = shader->get_id();
		near_clip_ = 0.1F;
		far_clip_ = 1000.F;
		fov_ = 45.F;
		projection_ = glm::perspective(fov_, (float)width_ / height_, near_clip_, far_clip_);
		camera_ = camera_system::get_default();
		ambient_colour_ = { 0.25F, 0.25F, 0.25F, 1.F };

		event::register_event(event_code::render_mode, this, on_event);

		std::string colour_3d_shader_name = "Shader.Builtin.Colour3DShader";
		auto colour_shader = resource_system::load(colour_3d_shader_name, egkr::resource_type::shader);
		auto properties = (shader::properties*)colour_shader->data;
		shader_system::create_shader(*properties);
		resource_system::unload(colour_shader);

		auto col_shader = shader_system::get_shader(colour_3d_shader_name);
		locations_.projection = col_shader->get_uniform_index("projection");
		locations_.view = col_shader->get_uniform_index("view");
		locations_.model = col_shader->get_uniform_index("model");
		return true;
	}
	bool render_view_world::on_destroy()
	{
		return true;
	}
	void render_view_world::on_resize(uint32_t width, uint32_t height)
	{
		if (width != width_ || height != height_)
		{
			width_ = width;
			height_ = height;
			projection_ = glm::perspective(fov_, (float)width_ / height_, near_clip_, far_clip_);

			for (auto& pass : renderpasses_)
			{
				pass->set_render_area(width_, height_);
			}
		}
	}
	render_view_packet render_view_world::on_build_packet(void* data)
	{
		mesh_packet_data* mesh_data = (mesh_packet_data*)data;

		render_view_packet packet{};

		packet.render_view = this;
		packet.projection_matrix = projection_;
		packet.view_matrix = camera_->get_view();
		packet.view_position = camera_->get_position();
		packet.ambient_colour = ambient_colour_;

		for (auto& mesh : mesh_data->meshes)
		{
			for (auto& geo : mesh->get_geometries())
			{
				if ((uint8_t)(geo->get_material()->get_diffuse_map()->texture->get_flags() & texture::flags::has_transparency) == 0)
				{
					packet.render_data.emplace_back(geo, mesh->get_model());
				}
			}
		}

		for (auto& [geo, model] : mesh_data->debug_meshes)
		{
			//for (auto& geo : mesh->get_geometries())
			{
				packet.debug_render_data.emplace_back(geo, model);
			}
		}

		return packet;
	}
	bool render_view_world::on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const
	{
		for (auto& pass : renderpasses_)
		{
			pass->begin(pass->get_render_targets()[render_target_index].get());

			shader_system::use(shader_id_);

			material_system::apply_global(shader_id_, render_view_packet->projection_matrix, render_view_packet->view_matrix, render_view_packet->ambient_colour, render_view_packet->view_position, mode_);
			for (egkr::geometry::render_data render_data : render_view_packet->render_data)
			{
				auto m = render_data.geometry->get_material();

				bool needs_update = m->get_render_frame() != frame_number;

				material_system::apply_instance(m, needs_update);
				m->set_render_frame(frame_number);

				material_system::apply_local(m, render_data.model.get_world());
				render_data.geometry->draw();

			}

			if (!render_view_packet->debug_render_data.empty())
			{
				auto colour_shader = shader_system::get_shader("Shader.Builtin.Colour3DShader");
				shader_system::use(colour_shader->get_id());
				shader_system::set_uniform(locations_.projection, &render_view_packet->projection_matrix);
				shader_system::set_uniform(locations_.view, &render_view_packet->view_matrix);
				shader_system::apply_global();
				for (egkr::geometry::render_data data : render_view_packet->debug_render_data)
				{
					const auto& model = data.model.get_world();
					shader_system::set_uniform(locations_.model, &model);
					data.geometry->draw();
				}

			}

			pass->end();

		}


		return true;
	}
}