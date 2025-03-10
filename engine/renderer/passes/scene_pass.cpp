#include "scene_pass.h"
#include "frame_data.h"
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

	{
	    const std::string shader_name = "Shader.PBR";
	    auto resource = resource_system::load(shader_name, resource::type::shader, nullptr);
	    auto* shader = (shader::properties*)resource->data;
	    shader_system::create_shader(*shader, renderpass.get());
	    resource_system::unload(resource);

	    pbr_shader = shader_system::get_shader(shader_name);

	    pbr_shader_locations.projection = pbr_shader->get_uniform_index("projection");
	    pbr_shader_locations.view = pbr_shader->get_uniform_index("view");
	    pbr_shader_locations.albedo_texture = pbr_shader->get_uniform_index("albedo_texture");
	    pbr_shader_locations.normal_texture = pbr_shader->get_uniform_index("normal_texture");
	    pbr_shader_locations.metallic_texture = pbr_shader->get_uniform_index("metallic_texture");
	    pbr_shader_locations.roughness_texture = pbr_shader->get_uniform_index("roughness_texture");
	    pbr_shader_locations.ao_texture = pbr_shader->get_uniform_index("ao_texture");
	    pbr_shader_locations.view_position = pbr_shader->get_uniform_index("view_position");
	    pbr_shader_locations.model = pbr_shader->get_uniform_index("model");
	    pbr_shader_locations.ibl_texture = pbr_shader->get_uniform_index("cube_texture");
	    pbr_shader_locations.shadow_texture = pbr_shader->get_uniform_index("shadow_texture");
	    pbr_shader_locations.light_space = pbr_shader->get_uniform_index("light_space");
	    pbr_shader_locations.point_lights = pbr_shader->get_uniform_index("point_lights");
	    pbr_shader_locations.directional_light = pbr_shader->get_uniform_index("dir_light");
	    pbr_shader_locations.num_point_lights = pbr_shader->get_uniform_index("num_point_lights");
	    pbr_shader_locations.ambient_colour = pbr_shader->get_uniform_index("ambient_colour");
	    pbr_shader_locations.properties = pbr_shader->get_uniform_index("properties");
	    pbr_shader_locations.mode = pbr_shader->get_uniform_index("mode");
	}


	event::register_event(event::code::render_mode, this, on_event);
	return true;
    }

    bool scene::execute(const frame_data& frame_data)
    {
	engine::get()->get_renderer()->set_active_viewport(viewport_);
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

	shader_system::use(pbr_shader->get_id());

	float4x4 light_space = data.directional_light_projection * data.directional_light_view;
	material_system::set_shadow_map(shadow_map_source->textures[frame_data.render_target_index], 0);

	shader_system::set_uniform(pbr_shader_locations.projection, &projection);
	shader_system::set_uniform(pbr_shader_locations.view, &view);
	shader_system::set_uniform(pbr_shader_locations.ambient_colour, &data.ambient_colour);
	shader_system::set_uniform(pbr_shader_locations.view_position, &view_position);
	shader_system::set_uniform(pbr_shader_locations.mode, &data.render_mode);
	shader_system::set_uniform(pbr_shader_locations.light_space, &light_space);
	pbr_shader->apply_globals(true);

	material::type current_type = material::type::pbr;

	for (const egkr::render_data& render_data : data.geometries)
	{
	    auto mat = render_data.render_geometry->get_material();
	    mat->get_shadow_map()->map_texture = material_system::get_shadow_map();
	    bool needs_update = (mat->get_render_frame() != frame_data.frame_number || mat->get_draw_index() != frame_data.draw_index);

	    if (current_type != mat->get_material_type())
	    {
		current_type = mat->get_material_type();
		shader_system::use(current_type == material::type::pbr ? pbr_shader->get_id() : material_shader->get_id());
	    }

	    switch (current_type)
	    {
	    case material::type::phong:
		material_system::apply_instance(mat, needs_update);
		break;
	    case material::type::pbr:
	    {
		mat->get_ibl_map()->map_texture = data.irradiance_texture;
		shader_system::bind_instance(mat->get_internal_id());
		shader_system::set_uniform(pbr_shader_locations.albedo_texture, &mat->get_albedo_map());
		shader_system::set_uniform(pbr_shader_locations.normal_texture, &mat->get_normal_map());
		shader_system::set_uniform(pbr_shader_locations.metallic_texture, &mat->get_metallic_map());
		shader_system::set_uniform(pbr_shader_locations.roughness_texture, &mat->get_roughness_map());
		shader_system::set_uniform(pbr_shader_locations.ao_texture, &mat->get_ao_map());
		shader_system::set_uniform(pbr_shader_locations.ibl_texture, &mat->get_ibl_map());
		shader_system::set_uniform(pbr_shader_locations.shadow_texture, &mat->get_shadow_map());
		shader_system::set_uniform(pbr_shader_locations.directional_light, light_system::get_directional_light());
		shader_system::set_uniform(pbr_shader_locations.point_lights, light_system::get_point_lights().data());
		auto num_point_lights = light_system::point_light_count();

		shader_system::set_uniform(pbr_shader_locations.num_point_lights, &num_point_lights);

		shader_system::apply_instance(needs_update);
		break;
	    }
	    default:
		LOG_ERROR("Unrecognised material type, skipping");
		continue;
	    }
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

    bool scene::load_resources()
    {
	for (auto& sink : sinks)
	{
	    if (sink.name == "shadowmap")
	    {
		shadow_map_source = sink.bound_source;
		break;
	    }
	}

	if (!shadow_map_source)
	{
	    LOG_ERROR("Could not find shadow map pass");
	    return false;
	}

	frame_count = engine::get()->get_renderer()->get_backend()->get_window_attachment_count();
	shadow_maps.resize(frame_count);
	for (auto i{0U}; i < frame_count; i++)
	{
	    auto& map = shadow_maps[i];
	    texture_map::properties props{
	        .minify = texture_map::filter::linear,
	        .magnify = texture_map::filter::linear,
	        .repeat_u = texture_map::repeat::clamp_to_edge,
	        .repeat_v = texture_map::repeat::clamp_to_edge,
	        .repeat_w = texture_map::repeat::clamp_to_edge,
	    };
	    map->update(props);
	    map->map_texture = shadow_map_source->textures[i];
	    map->acquire();
	}

	return true;
    }
}
