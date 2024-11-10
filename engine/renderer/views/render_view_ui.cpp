#include "render_view_ui.h"
#include <systems/shader_system.h>
#include <systems/material_system.h>
#include <systems/resource_system.h>
#include <renderer/renderer_types.h>
#include <renderer/renderer_frontend.h>

#include <resources/ui_text.h>
#include <resources/font.h>

namespace egkr
{
	render_view_ui::render_view_ui(const configuration& view_configuration)
		:render_view(view_configuration)
	{
	}

	bool render_view_ui::on_create()
	{
		auto ui_resource = resource_system::load("Shader.Builtin.UI", resource::type::shader, nullptr);
		auto ui_shader = (shader::properties*)ui_resource->data;

		shader_system::create_shader(*ui_shader, renderpasses_[0].get());

		resource_system::unload(ui_resource);
		shader_ = shader_system::get_shader("Shader.Builtin.UI");

		diffuse_map_location_ = shader_->get_uniform_index("diffuse_texture");
		diffuse_colour_location_ = shader_->get_uniform_index("diffuse_colour");
		model_location_ = shader_->get_uniform_index("model");

		view_ = float4x4{ 1.F };

		event::register_event(event::code::render_target_refresh_required, this, on_event);
		return true;
	}

	bool render_view_ui::on_destroy()
	{
		event::unregister_event(event::code::render_target_refresh_required, this, on_event);
		return true;
	}

	void render_view_ui::on_resize(uint32_t width, uint32_t height)
	{
		if (width_ != width || height_ != height)
		{
			width_ = width;
			height_ = height;
		}
	}

	render_view_packet render_view_ui::on_build_packet(void* data, const camera::shared_ptr& /*camera*/, viewport* viewport)
	{
		auto* ui_data = (ui_packet_data*)data;

		render_view_packet packet{};
		packet.view = this;
		packet.view_matrix = view_;
		packet.view_viewport = viewport;
		packet.extended_data = new ui_packet_data(*ui_data);

		for (const auto& msh : ui_data->mesh_data.meshes)
		{
			if (auto mesh = msh.lock())
			{
				for (const auto& geo : mesh->get_geometries())
				{
					packet.render_packet_data.emplace_back(geo, mesh);
				}
			}
		}

		return packet;
	}

	bool render_view_ui::on_render(render_view_packet* render_view_packet, const frame_data& frame_data) const
	{
		renderer->set_active_viewport(render_view_packet->view_viewport);

		for (const auto& pass : renderpasses_)
		{
			pass->begin(pass->get_render_targets()[frame_data.render_target_index].get());

			shader_system::use(shader_->get_id());

			material_system::apply_global(shader_->get_id(), frame_data, render_view_packet->view_viewport->projection, render_view_packet->view_matrix, {}, {}, 0);

			for (const auto& render_data : render_view_packet->render_packet_data)
			{
				auto m = render_data.render_geometry->get_material();

				bool needs_update = (m->get_render_frame() != frame_data.frame_number) || m->get_draw_index() != frame_data.draw_index;

				material_system::apply_instance(m, needs_update);
				m->set_render_frame(frame_data.frame_number);
				m->set_draw_index(frame_data.draw_index);

				if (auto transform = render_data.transform.lock())
				{
					material_system::apply_local(m, transform->get_world());
				}
				render_data.render_geometry->draw();
			}

			auto* texts = (ui_packet_data*)render_view_packet->extended_data;
			for (const auto& txt : texts->texts)
			{
				if (auto text = txt.lock())
				{
					shader_system::bind_instance(text->get_id());

					auto atlas = text->get_data()->atlas;
					if (atlas->map_texture->get_generation() == invalid_32_id)
					{
						continue;
					}
					shader_system::set_uniform(diffuse_map_location_, &text->get_data()->atlas);
					constexpr static const float4 white_colour{ 1, 1, 1, 1 };
					shader_system::set_uniform(diffuse_colour_location_, &white_colour);

					bool needs_update = (text->get_render_frame() != frame_data.frame_number) || (text->get_draw_index() != frame_data.draw_index);
					shader_system::apply_instance(needs_update);
					text->set_render_frame(frame_data.frame_number);
					text->set_draw_index(frame_data.draw_index);

					float4x4 model = text->get_world();

					shader_system::set_uniform(model_location_, &model);

					text->draw();
				}
			}

			pass->end();
		}
		return true;
	}
}
