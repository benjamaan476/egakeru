#include "render_view_skybox.h"

#include <renderer/renderer_frontend.h>
#include <renderer/renderpass.h>
#include <systems/shader_system.h>
#include <systems/material_system.h>
#include <systems/resource_system.h>

namespace egkr
{
	render_view_skybox::render_view_skybox(const configuration& configuration)
		: render_view(configuration)
	{
	}

	bool render_view_skybox::on_create()
	{
		auto skybox_shader_resource = resource_system::load("Shader.Builtin.Skybox", resource::type::shader, nullptr);
		shader_ = shader_system::create_shader(*(shader::properties*)skybox_shader_resource->data, renderpasses_[0].get());
		resource_system::unload(skybox_shader_resource);

//		shader_ = shader_system::get_shader("Shader.Builtin.Skybox");

		projection_ = glm::perspective(camera_->get_fov(), (float)width_ / (float)height_, camera_->get_near_clip(), camera_->get_far_clip());

		projection_location_ = shader_->get_uniform_index("projection");
		view_location_ = shader_->get_uniform_index("view");
		cube_map_location_ = shader_->get_uniform_index("cube_texture");

		event::register_event(event::code::render_target_refresh_required, this, on_event);
		return true;
	}

	bool render_view_skybox::on_destroy()
	{
		event::unregister_event(event::code::render_target_refresh_required, this, on_event);
		return true;
	}

	void render_view_skybox::on_resize(uint32_t width, uint32_t height)
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

	render_view_packet render_view_skybox::on_build_packet(void* data)
	{
		skybox_packet_data* skybox_data = (skybox_packet_data*)data;

		render_view_packet packet{};
		packet.render_view = this;
		packet.projection_matrix = projection_;
		packet.view_matrix = camera_->get_view();
		packet.view_position = camera_->get_position();
		if (auto skybox = skybox_data->skybox)
		{
			packet.render_data = { { skybox_data->skybox->get_geometry()}};
			packet.extended_data = new skybox_packet_data(*skybox_data);
		}
		return packet;
	}

	bool render_view_skybox::on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const
	{
		for (auto& pass : renderpasses_)
		{
			pass->begin(pass->get_render_target(render_target_index).get());
			if (skybox_packet_data* skybox_data = (skybox_packet_data*)render_view_packet->extended_data)
			{

				shader_system::use(shader_->get_id());
				float4x4 view = camera_->get_view();
				view[3][0] = 0.F;
				view[3][1] = 0.F;
				view[3][2] = 0.F;

				shader_->bind_globals();
				shader_system::set_uniform(projection_location_, &projection_);
				shader_system::set_uniform(view_location_, &view);
				bool needs_update = skybox_data->skybox->get_frame_number() != frame_number;
				shader_system::apply_global(needs_update);

				shader_system::bind_instance(skybox_data->skybox->get_instance_id());
				shader_system::set_uniform(cube_map_location_, &skybox_data->skybox->get_texture_map());

				shader_system::apply_instance(needs_update);
				skybox_data->skybox->set_frame_number(frame_number);

				render_view_packet->render_data[0].geometry->draw();
			}
			pass->end();
		}
		return true;
	}

	bool render_view_skybox::regenerate_attachment_target(uint32_t /*pass_index*/, const render_target::attachment& /*attachment*/)
	{
		return true;
	}
}

