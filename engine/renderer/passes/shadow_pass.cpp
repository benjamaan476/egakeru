#include "shadow_pass.h"
#include "engine/engine.h"
#include "pch.h"
#include "renderer/render_target.h"
#include "renderer/renderpass.h"
#include "renderer/viewport.h"
#include "systems/resource_system.h"
#include "systems/shader_system.h"
#include "systems/texture_system.h"
#include <iterator>
#include <system_error>

namespace egkr::pass
{
    shadow* shadow::create()
    {
	auto pass = new shadow();
	pass->init();
	return pass;
    }

    shadow::shadow(): resolution{1024} { }

    bool shadow::init()
    {
	name = "shadow";
	{
	    egkr::renderpass::configuration renderpass_configuration{
	        .name = name,
	        .clear_colour = {0, 0, 2, 1},
	        .pass_clear_flags = renderpass::clear_flags::depth | renderpass::clear_flags::stencil | renderpass::clear_flags::colour,
	        .depth = 1,
	        .stencil = 0,
	    };

	    render_target::attachment_configuration colour_attachment_configuration{
	        .type = render_target::attachment_type::colour,
	        .source = render_target::attachment_source::self,
	        .load_op = render_target::load_operation::dont_care,
	        .store_op = render_target::store_operation::store,
	        .present_after = false,
	    };

	    render_target::attachment_configuration depth_attachment_configuration{.type = render_target::attachment_type::depth,
	        .source = render_target::attachment_source::self,
	        .load_op = render_target::load_operation::dont_care,
	        .store_op = render_target::store_operation::store,
	        .present_after = true};

	    renderpass_configuration.target.attachments.push_back(colour_attachment_configuration);
	    renderpass_configuration.target.attachments.push_back(depth_attachment_configuration);

	    renderpass = renderpass::renderpass::create(renderpass_configuration);
	}

	{
	    const std::string shader_name = "Shader.Shadow";
	    auto resource = resource_system::load(shader_name, resource::type::shader, nullptr);
	    auto* shader = (shader::properties*)resource->data;
	    shader_system::create_shader(*shader, renderpass.get());
	    resource_system::unload(resource);

	    shadow_shader = shader_system::get_shader(shader_name);

	    shadow_shader_locations.projection = shadow_shader->get_uniform_index("projection");
	    shadow_shader_locations.view = shadow_shader->get_uniform_index("view");
	    shadow_shader_locations.model = shadow_shader->get_uniform_index("model");
	    // shadow_shader_locations.light_space = shadow_shader->get_uniform_index("light_space");
	    shadow_shader_locations.colour_map = shadow_shader->get_uniform_index("colour_texture");
	}

	for (auto i{0U}; i < egkr::engine::get()->get_renderer()->get_backend()->get_window_attachment_count(); i++)
	{
	}

	return true;
    }

    bool shadow::load_resources()
    {
	texture_map::properties props{
	    .minify = texture_map::filter::linear,
	    .magnify = texture_map::filter::linear,
	    .repeat_u = texture_map::repeat::repeat,
	    .repeat_v = texture_map::repeat::repeat,
	    .repeat_w = texture_map::repeat::repeat,
	};
	default_colour_map = texture_map::create(props);
	default_colour_map->map_texture = texture_system::get_default_diffuse_texture();
	default_colour_map->acquire();

	shadow_shader->acquire_instance_resources({default_colour_map});

	float near_clip = -400;
	float far_clip = 400;
	float fov = 0;
	viewport_ = new viewport({0, 0, resolution, resolution}, projection_type::orthographic, fov, near_clip, far_clip);
	float area = 20;
	projection = glm::ortho(-area, area, -area, area, near_clip, far_clip);

	return true;
    }

    bool shadow::execute(const frame_data& frame_data)
    {
	engine::get()->get_renderer()->set_active_viewport(viewport_);
	renderpass->begin(frame_data.render_target_index);

	shader_system::use(shadow_shader->get_id());
	shader_system::set_uniform(shadow_shader_locations.projection, &projection);
	shader_system::set_uniform(shadow_shader_locations.view, &view);
	shader_system::apply_global(true);

	uint32_t highest_count = 0;
	for (const auto& geo : data.geometries)
	{
	    auto& mat = geo.render_geometry->get_material();
	    if (mat->get_id() > highest_count)
	    {
		highest_count = mat->get_id() + 1;
	    }
	}
	highest_count++;

	if (highest_count > instance_count)
	{
	    for (auto i{0U}; i < instance_count; i++)
	    {
		const auto& map = default_colour_map;
		shadow_shader->acquire_instance_resources({map});
	    }
	    instance_count = highest_count;
	}

	for (const auto& geo : data.geometries)
	{
	    uint32_t bind_index = invalid_32_id;
	    texture_map::shared_ptr bind_map;
	    uint64_t render_number = 0;
	    uint8_t draw_index = invalid_8_id;

	    if (const auto& mat = geo.render_geometry->get_material())
	    {
		bind_index = mat->get_internal_id() + 1;
		bind_map = mat->get_diffuse_map();
		render_number = shadow_shader->get_frame_number();
		draw_index = (uint8_t)shadow_shader->get_draw_index();
	    }

	    bool needs_update = render_number != frame_data.render_target_index || draw_index != frame_data.draw_index;
	    shader_system::bind_instance(bind_index);
	    shader_system::set_uniform(shadow_shader_locations.colour_map, &bind_map);
	    shader_system::apply_instance(needs_update);

	    if (geo.is_winding_reversed)
	    {
		engine::get()->get_renderer()->set_winding(winding::clockwise);
	    }
	    geo.render_geometry->draw();
	    if (geo.is_winding_reversed)
	    {
		engine::get()->get_renderer()->set_winding(winding::counter_clockwise);
	    }
	}
	return true;
    }

    texture::shared_ptr shadow::get_texture(render_target::attachment_type type, uint8_t index) const
    {
	if (type == render_target::attachment_type::colour)
	{
	    return colour_textures[index];
	}
	else if (type == render_target::attachment_type::depth)
	{
	    return depth_textures[index];
	}

	LOG_ERROR("Unrecognised attachment type");
	return nullptr;
    }

    bool shadow::destroy()
    {
	shadow_shader->free();
	shadow_shader.reset();
	return true;
    }
}
