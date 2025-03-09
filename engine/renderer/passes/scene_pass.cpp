#include "scene_pass.h"
#include "renderer/renderpass.h"
#include <systems/resource_system.h>
#include <systems/shader_system.h>
#include <systems/material_system.h>

#include "engine/engine.h"
#include "systems/light_system.h"

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
	{
	    std::string terrain_shader_name = "Shader.Terrain";
	    auto shader = resource_system::load(terrain_shader_name, egkr::resource::type::shader, nullptr);
	    auto* properties = (shader::properties*)shader->data;
	    shader_system::create_shader(*properties, renderpass.get());
	    resource_system::unload(shader);

	    terrain_shader = shader_system::get_shader(terrain_shader_name);
	    terrain_shader->acquire_instance_resources({});

	    terrain_shader_locations.projection = terrain_shader->get_uniform_index("projection");
	    terrain_shader_locations.view = terrain_shader->get_uniform_index("view");
	    terrain_shader_locations.shininess = terrain_shader->get_uniform_index("shininess");
	    terrain_shader_locations.ambient_colour = terrain_shader->get_uniform_index("ambient_colour");
	    terrain_shader_locations.diffuse_colour = terrain_shader->get_uniform_index("diffuse_colour");
	    // terrain_shader_locations.diffuse_texture = terrain_shader->get_uniform_index("diffuse_texture");
	    // terrain_shader_locations.specular_texture = terrain_shader->get_uniform_index("specular_texture");
	    // terrain_shader_locations.normal_texture = terrain_shader->get_uniform_index("normal_texture");
	    terrain_shader_locations.view_position = terrain_shader->get_uniform_index("view_position");
	    terrain_shader_locations.model = terrain_shader->get_uniform_index("model");
	    terrain_shader_locations.mode = terrain_shader->get_uniform_index("mode");
	    terrain_shader_locations.point_light = terrain_shader->get_uniform_index("point_lights");
	    terrain_shader_locations.directional_light = terrain_shader->get_uniform_index("dir_light");
	    terrain_shader_locations.num_point_lights = terrain_shader->get_uniform_index("num_point_lights");
	}

	event::register_event(event::code::render_mode, this, on_event);
	return true;
    }

    bool scene::execute(const frame_data& frame_data) const
    {
	engine::get()->get_renderer()->set_active_viewport(viewport);
	renderpass->begin(frame_data.render_target_index);

	if (!data.terrain.empty())
	{
	    const float shininess{32};
	    const float4 diffuse_colour{0.5, 0.5, 0.5, 1.0};
	    shader_system::use(terrain_shader->get_id());
	    shader_system::set_uniform(terrain_shader_locations.projection, &projection);
	    shader_system::set_uniform(terrain_shader_locations.view, &view);
	    shader_system::set_uniform(terrain_shader_locations.ambient_colour, &data.ambient_colour);
	    shader_system::set_uniform(terrain_shader_locations.view_position, &view_position);
	    shader_system::set_uniform(terrain_shader_locations.mode, &data.render_mode);

	    shader_system::apply_global(true);

	    shader_system::bind_instance(0);
	    shader_system::set_uniform(terrain_shader_locations.diffuse_colour, &diffuse_colour);
	    shader_system::set_uniform(terrain_shader_locations.directional_light, light_system::get_directional_light());
	    shader_system::set_uniform(terrain_shader_locations.point_light, light_system::get_point_lights().data());
	    auto num_point_lights = light_system::point_light_count();
	    shader_system::set_uniform(terrain_shader_locations.num_point_lights, &num_point_lights);
	    shader_system::set_uniform(terrain_shader_locations.shininess, &shininess);
	    shader_system::apply_instance(true);

	    for (const auto& terrain : data.terrain)
	    {
		if (const auto& world = terrain.transform.lock())
		{
		    const auto& model = world->get_world();
		    shader_system::set_uniform(terrain_shader_locations.model, &model);
		}
	    }

	    if (data.terrain.front().is_winding_reversed)
	    {
		engine::get()->get_renderer()->set_winding(winding::clockwise);
	    }

	    data.terrain.front().render_geometry->draw();

	    if (data.terrain.front().is_winding_reversed)
	    {
		engine::get()->get_renderer()->set_winding(winding::counter_clockwise);
	    }
	}

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
		engine::get()->get_renderer()->set_winding(winding::clockwise);
	    }
	    render_data.render_geometry->draw();
	    if (render_data.is_winding_reversed)
	    {
		engine::get()->get_renderer()->set_winding(winding::counter_clockwise);
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
	default:
	    return false;
	}
	return false;
    }
}
