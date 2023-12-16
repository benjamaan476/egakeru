#include "render_view_ui.h"
#include <systems/shader_system.h>
#include <systems/material_system.h>
#include <renderer/renderer_types.h>

namespace egkr::render_view
{
	render_view_ui::render_view_ui(const renderer_frontend* renderer, const configuration& configuration)
		:render_view(renderer, configuration)
	{}
	bool render_view_ui::on_create()
	{
		auto shader = shader_system::get_shader(custom_shader_name_ != "" ? custom_shader_name_ : BUILTIN_SHADER_NAME_UI);
		shader_id_ = shader->get_id();
		near_clip_ = -100.F;
		far_clip_ = 100.F;
		projection_ = glm::ortho(0.F, (float)width_, (float)height_, 0.F, near_clip_, far_clip_);
		view_ = float4x4{ 1.F };
		return true;
	}

	bool render_view_ui::on_destroy()
	{
		return true;
	}

	void render_view_ui::on_resize(uint32_t width, uint32_t height)
	{
		if (width_ != width || height_ != height)
		{
			width_ = width;
			height_ = height;
			projection_ = glm::ortho(0.F, (float)width_, (float)height_, 0.F, near_clip_, far_clip_);

			for (auto& pass : renderpasses_)
			{
				pass->set_render_area(width_, height_);
			}
	}
	}

	render_view_packet render_view_ui::on_build_packet(void* data)
	{
		mesh_packet_data* mesh_data = (mesh_packet_data*)data;

		render_view_packet packet{};
		packet.render_view = this;
		packet.projection_matrix = projection_;
		packet.view_matrix = view_;

		for (const auto& mesh : mesh_data->meshes)
		{
			for (const auto& geo : mesh->get_geometries())
			{
				packet.render_data.emplace_back(geo, mesh->get_model());
			}
		}
		return packet;
	}

	bool render_view_ui::on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const
	{
		for (auto& pass : renderpasses_)
		{
			pass->begin(pass->get_render_targets()[render_target_index].get());

			shader_system::use(shader_id_);

			material_system::apply_global(shader_id_, render_view_packet->projection_matrix, render_view_packet->view_matrix, {}, {}, 0);

			for (auto render_data : render_view_packet->render_data)
			{
				auto m = render_data.geometry->get_material();

				bool needs_update = m->get_render_frame() != frame_number;

				material_system::apply_instance(m, needs_update);
				m->set_render_frame(frame_number);

				material_system::apply_local(m, render_data.model.get_world());
				render_data.geometry->draw();

			}

			pass->end();
		}
		return true;
	}

}
