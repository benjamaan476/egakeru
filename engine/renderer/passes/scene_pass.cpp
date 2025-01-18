#include "scene_pass.h"
#include "renderer/renderpass.h"
#include <systems/resource_system.h>
#include <systems/shader_system.h>
#include <systems/material_system.h>

#include <renderer/renderer_frontend.h>

namespace egkr::pass
{
    scene* scene::create()
    {
	auto pass = new scene();
	pass->init();
	return pass;
    }


    bool scene::init()
    {
	name = "scene";
	{
	    egkr::renderpass::configuration renderpass_configuration{
	        .name = name, .clear_colour = {0, 0, 0.2F, 1.F}, .pass_clear_flags = egkr::renderpass::clear_flags::depth | egkr::renderpass::clear_flags::stencil, .depth = 1.F, .stencil = 0};

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

	    renderpass = egkr::renderpass::renderpass::create(renderpass_configuration);
	}

	// egkr::render_view::configuration opaque_world{};
	// opaque_world.view_type = egkr::render_view::type::world;
	// opaque_world.width = width_;
	// opaque_world.height = height_;
	// opaque_world.name = "world-opaque";
	// opaque_world.passes.push_back(skybox_renderpass_configuration);
	// opaque_world.passes.push_back(renderpass_configuration);
	// opaque_world.view_source = egkr::render_view::view_matrix_source::scene_camera;

	{
	    const std::string shader_name = "Shader.Material";
	    auto resource = resource_system::load(shader_name, resource::type::shader, nullptr);
	    auto* shader = (shader::properties*)resource->data;
	    shader_system::create_shader(*shader, renderpass.get());
	    resource_system::unload(resource);

	    material_shader = shader_system::get_shader(shader_name);
	}

	{
	    std::string colour_3d_shader_name = "Shader.Colour3DShader";
	    auto shader = resource_system::load(colour_3d_shader_name, egkr::resource::type::shader, nullptr);
	    auto* properties = (shader::properties*)shader->data;
	    shader_system::create_shader(*properties, renderpass.get());
	    resource_system::unload(shader);

	    debug_colour_shader = shader_system::get_shader(colour_3d_shader_name);
	    debug_shader_locations.projection = debug_colour_shader->get_uniform_index("projection");
	    debug_shader_locations.view = debug_colour_shader->get_uniform_index("view");
	    debug_shader_locations.model = debug_colour_shader->get_uniform_index("model");
	}

	event::register_event(event::code::render_mode, this, on_event);
	event::register_event(event::code::render_target_refresh_required, this, on_event);
	return true;
    }

    bool scene::execute(const frame_data& frame_data) const
    {
	renderer->set_active_viewport(viewport);
	renderpass->begin(frame_data.render_target_index);

	shader_system::use(material_shader->get_id());
	material_system::apply_global(material_shader->get_id(), frame_data, projection, view, data.ambient_colour, view_position, data.render_mode);

	for (const egkr::render_data& render_data : data.geometries)
	{
	    auto mat = render_data.render_geometry->get_material();
	    bool needs_update = (mat->get_render_frame() != frame_data.frame_number || mat->get_draw_index() != frame_data.draw_index);

	    material_system::apply_instance(mat, needs_update);
	    mat->set_render_frame(frame_data.frame_number);
	    mat->set_draw_index(frame_data.draw_index);

	    if (auto transform = render_data.transform.lock())
	    {
		material_system::apply_local(mat, transform->get_world());
	    }

	    if (render_data.is_winding_reversed)
	    {
		renderer->set_winding(winding::clockwise);
	    }
	    render_data.render_geometry->draw();
	    if (render_data.is_winding_reversed)
	    {
		renderer->set_winding(winding::counter_clockwise);
	    }
	}

	if (!data.debug_geometries.empty())
	{
	    shader_system::use(debug_colour_shader->get_id());
	    shader_system::set_uniform(debug_shader_locations.projection, &projection);
	    shader_system::set_uniform(debug_shader_locations.view, &view);
	    shader_system::apply_global(true);
	    for (const render_data& debug_data : data.debug_geometries)
	    {
		if (auto transform = debug_data.transform.lock())
		{
		    const auto& model = transform->get_world();
		    shader_system::set_uniform(debug_shader_locations.model, &model);
		}
		debug_data.render_geometry->draw();
	    }

	    debug_colour_shader->set_frame_number(frame_data.frame_number);
	    debug_colour_shader->set_draw_index(frame_data.draw_index);
	}

	renderpass->end();
	return true;
    }

    bool scene::destroy()
    {
	material_shader->free();
	material_shader.reset();

	debug_colour_shader->free();
	debug_colour_shader.reset();

	renderpass->free();
	renderpass.reset();

	event::unregister_event(event::code::render_target_refresh_required, this, on_event);
	event::unregister_event(event::code::render_mode, this, on_event);
	return true;
    }

    bool scene::on_event(event::code code, void* /*sender*/, void* listener, const event::context& context)
    {
	auto* view = (scene*)listener;

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
