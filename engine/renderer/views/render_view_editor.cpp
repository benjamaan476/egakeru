#include "render_view_editor.h"
#include <systems/resource_system.h>
#include <systems/shader_system.h>
#include <systems/material_system.h>
#include <renderer/renderer_types.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace egkr
{
	render_view_editor::render_view_editor(const configuration& configuration) : render_view(configuration)
	{}

	bool render_view_editor::on_create()
	{
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

	bool render_view_editor::on_destroy()
	{
		event::unregister_event(event::code::render_target_refresh_required, this, on_event);
		return true;
	}

	void render_view_editor::on_resize(uint32_t width, uint32_t height)
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
	render_view_packet render_view_editor::on_build_packet(void* data)
	{
		render_view_packet packet{};

		packet.render_view = this;
		packet.projection_matrix = projection_;
		packet.view_matrix = camera_->get_view();
		packet.view_position = camera_->get_position();
		packet.ambient_colour = ambient_colour_;
		gizmo_ = *(editor::gizmo*)data;

		return packet;
	}

	bool render_view_editor::on_render(const render_view_packet* render_view_packet, uint32_t  frame_number , uint32_t render_target_index) const
	{
		for (auto& pass : renderpasses_)
		{
			pass->begin(pass->get_render_targets()[render_target_index].get());

			auto colour_shader = shader_system::get_shader("Shader.Builtin.Colour3DShader");
			shader_system::use(colour_shader->get_id());
			colour_shader->bind_globals();
			bool needs_update = frame_number != colour_shader->get_frame_number();
			if (needs_update)
			{
				shader_system::set_uniform(locations_.projection, &render_view_packet->projection_matrix);
				shader_system::set_uniform(locations_.view, &render_view_packet->view_matrix);
			}
			shader_system::apply_global(needs_update);

			const auto& camera_position = camera_->get_position();
			const auto& gizmo_position = gizmo_.get_position();

			const auto distance = glm::distance(camera_position, gizmo_position);

			gizmo_.get_selected_model()
				.and_then([&](float4x4 model) -> std::optional<float4x4>
						  {
							  float3 model_scale, translation, skew;
							  float4 perspective;
							  glm::quat rotation;
							  glm::decompose(model, model_scale, rotation, translation, skew, perspective);
							  const float fixed_size{ 0.1F };
							  auto scale_factor = (2 * std::tan(camera_->get_fov() / 2) * distance) * fixed_size;
							  editor::gizmo::set_scale(scale_factor);
							  float4x4 scale = glm::scale(glm::mat4x4(1.f), { scale_factor, scale_factor, scale_factor });

							  float4x4 local{ 1.F };
							  auto trans = glm::translate(local, translation);
							  auto rot = glm::toMat4(rotation);

							  model = trans * rot * scale;
							  shader_system::set_uniform(locations_.model, &model);
							  gizmo_.draw();

							  return model;
						  });

			pass->end();
		}
		return true;
	}

	bool render_view_editor::regenerate_attachment_target(uint32_t /*pass_index*/, const render_target::attachment& /*attachment*/)
	{
		return true;
	}
}
