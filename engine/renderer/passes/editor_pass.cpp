#include "editor_pass.h"
#include <glm/geometric.hpp>
#include <systems/resource_system.h>
#include <systems/shader_system.h>

#include <renderer/renderer_frontend.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
namespace egkr::pass
{
    bool editor::init()
    {
	{
	    egkr::renderpass::configuration renderpass_configuration{
	        .name = "Renderpass.World", .clear_colour = {0, 0, 0.2F, 1.F}, .pass_clear_flags = egkr::renderpass::clear_flags::depth | egkr::renderpass::clear_flags::stencil, .depth = 1.F, .stencil = 0};

	    egkr::render_target::attachment_configuration colour_attachment_configration{.type = egkr::render_target::attachment_type::colour,
	        .source = egkr::render_target::attachment_source::default_source,
	        .load_op = egkr::render_target::load_operation::load,
	        .store_op = egkr::render_target::store_operation::store,
	        .present_after = false};


	    egkr::render_target::attachment_configuration depth_attachment_configration{.type = egkr::render_target::attachment_type::depth,
	        .source = egkr::render_target::attachment_source::default_source,
	        .load_op = egkr::render_target::load_operation::dont_care,
	        .store_op = egkr::render_target::store_operation::store,
	        .present_after = false};

	    renderpass_configuration.target.attachments.push_back(colour_attachment_configration);
	    renderpass_configuration.target.attachments.push_back(depth_attachment_configration);

	    // egkr::render_view::configuration opaque_world{};
	    // opaque_world.view_type = egkr::render_view::type::editor;
	    // opaque_world.width = width_;
	    // opaque_world.height = height_;
	    // opaque_world.name = "editor";
	    // opaque_world.passes.push_back(renderpass_configuration);
	    // opaque_world.view_source = egkr::render_view::view_matrix_source::scene_camera;

	    renderpass->create(renderpass_configuration);
	}

	std::string colour_3d_shader_name = "Shader.Colour3DShader";
	auto shader = resource_system::load(colour_3d_shader_name, egkr::resource::type::shader, nullptr);
	auto* properties = (shader::properties*)shader->data;
	shader_system::create_shader(*properties, renderpass.get());
	resource_system::unload(colour_shader);

	colour_shader = shader_system::get_shader(colour_3d_shader_name);
	debug_shader_locations.projection = colour_shader->get_uniform_index("projection");
	debug_shader_locations.view = colour_shader->get_uniform_index("view");
	debug_shader_locations.model = colour_shader->get_uniform_index("model");

	event::register_event(event::code::render_target_refresh_required, this, on_event);
	return true;
    }

    bool editor::destroy()
    {
	event::unregister_event(event::code::render_target_refresh_required, this, on_event);
	return true;
    }

    bool editor::execute(const frame_data& frame_data) const
    {
	renderer->set_active_viewport(viewport);

	renderpass->begin(frame_data.render_target_index);

	shader_system::use(colour_shader->get_id());
	colour_shader->bind_globals();

	bool needs_update = (frame_data.frame_number != colour_shader->get_frame_number()) || (frame_data.draw_index != colour_shader->get_draw_index());
	if (needs_update)
	{
	    shader_system::set_uniform(debug_shader_locations.projection, &projection);
	    shader_system::set_uniform(debug_shader_locations.view, &view);
	}
	shader_system::apply_global(needs_update);

	const auto& gizmo_position = data.gizmo.get_position();
	const auto distance = glm::distance(view_position, gizmo_position);

	data.gizmo.get_selected_model().and_then(
	    [&](float4x4 model) -> std::optional<float4x4>
	    {
	        float3 model_scale;
	        float3 translation;
	        float3 skew;
	        float4 perspective;
	        glm::quat rotation;
	        glm::decompose(model, model_scale, rotation, translation, skew, perspective);

	        constexpr const float fixed_size{0.1f};
	        auto scale_factor = (2 * std::tan(viewport->fov / 2) * distance) * fixed_size;
	        egkr::editor::gizmo::set_scale(scale_factor);
	        float4x4 scale = glm::scale(glm::mat4(1.f), glm::vec3{scale_factor, scale_factor, scale_factor});

	        float4x4 local{1.f};
	        auto trans = glm::translate(local, translation);
	        auto rot = glm::toMat4(rotation);

	        model = trans * rot * scale;
	        shader_system::set_uniform(debug_shader_locations.model, &model);
	        data.gizmo.draw();

	        return model;
	    });

	renderpass->end();
	return true;
    }

    bool editor::on_event(event::code code, void* /*sender*/, void* listener, const event::context& context)
    {
	auto* view = (editor*)listener;

	switch (code)
	{
	case event::code::render_mode:
	{
	    context.get(0, view->data.render_mode);
	}
	break;
	case event::code::render_target_refresh_required:
	    view->regenerate_render_targets();
	    break;
	default:
	    return false;
	}
	return false;
    }
}
