#include "render_view_skybox.h"

#include <systems/shader_system.h>
#include <systems/material_system.h>

namespace egkr::render_view
{
render_view_skybox::render_view_skybox(const renderer_frontend* renderer, const configuration& configuration)
	: render_view(renderer, configuration)
{}

bool render_view_skybox::on_create()
{
	auto shader = shader_system::get_shader(custom_shader_name_ != "" ? custom_shader_name_ : BUILTIN_SHADER_NAME_SKYBOX);
	shader_id_ = shader->get_id();

	projection_ = glm::perspective(fov_, (float)width_ / height_, near_clip_, far_clip_);

	projection_location_ = shader->get_uniform_index("projection");
	view_location_ = shader->get_uniform_index("view");
	cube_map_location_ = shader->get_uniform_index("cube_texture");

	return false;
}

bool render_view_skybox::on_destroy()
{
	return false;
}

void render_view_skybox::on_resize(uint32_t width, uint32_t height)
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

render_view_packet render_view_skybox::on_build_packet(void* data)
{
	skybox_packet_data* skybox_data = (skybox_packet_data*)data;

	render_view_packet packet{};
	packet.render_view = this;
	packet.projection_matrix = projection_;
	packet.view_matrix = camera_->get_view();
	packet.view_position = camera_->get_position();
	packet.render_data = { { skybox_data->skybox->get_geometry() } };
	packet.extended_data = skybox_data;
	return packet;
}

bool render_view_skybox::on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const
{
	for (auto& pass : renderpasses_)
	{
		pass->begin(pass->get_render_target(render_target_index).get());
		shader_system::use(shader_id_);

		float4x4 view = camera_->get_view();
		view[3][0] = 0.F;
		view[3][1] = 0.F;
		view[3][2] = 0.F;

		auto shader = shader_system::get_shader(shader_id_);
		shader->bind_globals();

		shader_system::set_uniform(projection_location_, &projection_);
		shader_system::set_uniform(view_location_, &view);
		shader_system::apply_global();

		shader->bind_instances(0);
		skybox_packet_data* skybox_data = (skybox_packet_data*)render_view_packet->extended_data;
		shader_system::set_uniform(cube_map_location_, skybox_data->skybox->get_texture_map().get());
		
		bool needs_update = skybox_data->skybox->get_frame_number() != frame_number;
		shader_system::apply_instance(needs_update);
		skybox_data->skybox->set_frame_number(frame_number);

		render_view_packet->render_data[0].geometry->draw();

		pass->end();
	}
	return true;
}

}

