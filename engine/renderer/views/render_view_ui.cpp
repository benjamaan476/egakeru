#include "render_view_ui.h"
#include <systems/shader_system.h>
#include <systems/material_system.h>
#include <systems/resource_system.h>
#include <renderer/renderer_types.h>

#include <resources/ui_text.h>
#include <resources/font.h>

namespace egkr::render_view
{
	render_view_ui::render_view_ui(const configuration& configuration)
		:render_view(configuration)
	{
		regenerate_render_targets();
	}

	bool render_view_ui::on_create()
	{
		auto ui_resource = resource_system::load("Shader.Builtin.UI", resource_type::shader, nullptr);
		auto ui_shader = (shader::properties*)ui_resource->data;

		shader_system::create_shader(*ui_shader, renderpasses_[0].get());

		resource_system::unload(ui_resource);
		shader_ = shader_system::get_shader("Shader.Builtin.UI");

		diffuse_map_location_ = shader_->get_uniform_index("diffuse_texture");
		diffuse_colour_location_ = shader_->get_uniform_index("diffuse_colour");
		model_location_ = shader_->get_uniform_index("model");

		near_clip_ = -100.F;
		far_clip_ = 100.F;
		projection_ = glm::ortho(0.F, (float)width_, (float)height_, 0.F, near_clip_, far_clip_);
		view_ = float4x4{ 1.F };

		event::register_event(event_code::render_target_refresh_required, this, on_event);
		return true;
	}

	bool render_view_ui::on_destroy()
	{
		event::unregister_event(event_code::render_target_refresh_required, this, on_event);
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
		ui_packet_data* ui_data = (ui_packet_data*)data;

		render_view_packet packet{};
		packet.render_view = this;
		packet.projection_matrix = projection_;
		packet.view_matrix = view_;
		packet.extended_data = new ui_packet_data(*ui_data);

		for (const auto& mesh : ui_data->mesh_data.meshes)
		{
			for (const auto& geo : mesh->get_geometries())
			{
				packet.render_data.emplace_back(geo, mesh->get_model());
			}
		}

		return packet;
	}

	bool render_view_ui::on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index)
	{
		for (auto& pass : renderpasses_)
		{
			pass->begin(pass->get_render_targets()[render_target_index].get());

			shader_system::use(shader_->get_id());

			material_system::apply_global(shader_->get_id(), render_view_packet->projection_matrix, render_view_packet->view_matrix, {}, {}, 0);

			for (auto render_data : render_view_packet->render_data)
			{
				auto& m = render_data.geometry->get_material();

				bool needs_update = m->get_render_frame() != frame_number;
				material_system::apply_instance(m, needs_update);
				m->set_render_frame(frame_number);

				material_system::apply_local(m, render_data.model.get_world());
				render_data.geometry->draw();

				auto* texts = (ui_packet_data*)render_view_packet->extended_data;
				for (const auto& text : texts->texts)
				{
					shader_system::bind_instance(text->get_id());

					shader_system::set_uniform(diffuse_map_location_, &text->get_data()->atlas);
					constexpr static const float4 white_colour{ 1, 1, 1, 1 };
					shader_system::set_uniform(diffuse_colour_location_, &white_colour);

					bool needs_update = text->get_render_frame() != frame_number;
					shader_system::apply_instance(needs_update);
					text->set_render_frame(frame_number);

					float4x4 model = text->get_transform().get_world();

					shader_system::set_uniform(model_location_, &model);

					text->draw();
				}
			}

			pass->end();
		}
		return true;
	}

	bool render_view_ui::regenerate_attachment_target(uint32_t /*pass_index*/, render_target::attachment& /*attachment*/)
	{
		return true;
	}
}
