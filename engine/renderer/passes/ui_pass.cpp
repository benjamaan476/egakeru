#include "ui_pass.h"
#include "frame_data.h"
#include "pch.h"
#include "resources/geometry.h"
#include "resources/shader.h"
#include <systems/resource_system.h>
#include <systems/shader_system.h>
#include <resources/ui_text.h>
#include <resources/font.h>

#include "renderer/renderer_frontend.h"
#include "systems/material_system.h"

namespace egkr::pass
{
    ui* ui::create()
    {
	auto pass = new ui();
	pass->init();
	return pass;
    }
    bool ui::init()
    {
	name = "ui";
	{
	    egkr::renderpass::configuration renderpass_configuration{.name = name, .clear_colour = {0, 0, 0.2F, 1.F}, .pass_clear_flags = egkr::renderpass::clear_flags::none, .depth = 1.F, .stencil = 0};

	    egkr::render_target::attachment_configuration attachment_configration{.type = egkr::render_target::attachment_type::colour,
	        .source = egkr::render_target::attachment_source::default_source,
	        .load_op = egkr::render_target::load_operation::load,
	        .store_op = egkr::render_target::store_operation::store,
	        .present_after = true};

	    renderpass_configuration.target.attachments.push_back(attachment_configration);

	    renderpass = renderpass::renderpass::create(renderpass_configuration);
	}

	auto ui_resource = resource_system::load("Shader.UI", resource::type::shader, nullptr);
	auto ui_shader = (shader::properties*)ui_resource->data;

	shader_system::create_shader(*ui_shader, renderpass.get());

	resource_system::unload(ui_resource);
	shader = shader_system::get_shader("Shader.UI");

	shader_locations.diffuse_map_location = shader->get_uniform_index("diffuse_texture");
	shader_locations.diffuse_colour_location = shader->get_uniform_index("diffuse_colour");
	shader_locations.model_location = shader->get_uniform_index("model");

	return true;
    }

    bool ui::execute(const frame_data& frame_data) const
    {
	renderer->set_active_viewport(viewport);

	renderpass->begin(frame_data.render_target_index);

	shader_system::use(shader->get_id());
	material_system::apply_global(shader->get_id(), frame_data, projection, view, {}, {}, 0);

	for (const auto& render_data : data.ui_geometries)
	{
	    auto mat = render_data.render_geometry->get_material();

	    bool needs_update = (mat->get_render_frame() != frame_data.frame_number) || mat->get_draw_index() != frame_data.draw_index;
	    material_system::apply_instance(mat, needs_update);
	    mat->set_render_frame(frame_data.frame_number);
	    mat->set_draw_index(frame_data.draw_index);

	    if (auto transform = render_data.transform.lock())
	    {
		material_system::apply_local(mat, transform->get_world());
	    }
	    render_data.render_geometry->draw();
	}

	for (const auto& txt : data.texts)
	{
	    if (auto text = txt.lock())
	    {
		shader_system::bind_instance(text->get_id());

		auto atlas = text->get_data()->atlas;
		if (atlas->map_texture->get_generation() == invalid_32_id)
		{
		    continue;
		}

		shader_system::set_uniform(shader_locations.diffuse_map_location, &text->get_data()->atlas);
		constexpr static const float4 white_colour{1, 1, 1, 1};
		shader_system::set_uniform(shader_locations.diffuse_colour_location, &white_colour);

		bool needs_update = (text->get_render_frame() != frame_data.frame_number) || (text->get_draw_index() != frame_data.draw_index);
		shader_system::apply_instance(needs_update);
		text->set_render_frame(frame_data.render_target_index);
		text->set_draw_index(frame_data.draw_index);

		float4x4 model = text->get_world();
		shader_system::set_uniform(shader_locations.model_location, &model);
		text->draw();
	    }
	}

	renderpass->end();
	return true;
    }

    bool ui::destroy()
    {
	renderpass->free();
	renderpass.reset();
	return true;
    }
}
