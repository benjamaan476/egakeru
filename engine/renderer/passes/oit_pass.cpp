#include "oit_pass.h"
#include "engine/engine.h"

#include <systems/resource_system.h>
#include <systems/shader_system.h>

namespace egkr::pass
{
    oit* oit::create()
    {
	auto pass = new oit();
	pass->init();
	return pass;
    }

    bool oit::init()
    {
	name = "oit";
	{
	    egkr::renderpass::configuration renderpass_configuration{
		.name = name,
		.clear_colour = {},
		.pass_clear_flags = renderpass::clear_flags::depth | renderpass::clear_flags::stencil,
		.depth = 1.f,
		.stencil = 0
	    };

	    render_target::attachment_configuration colour_attachment_configuration{
		.type = render_target::attachment_type::colour,
		.source = render_target::attachment_source::default_source,
		.load_op = render_target::load_operation::load,
		.store_op = render_target::store_operation::store,
		.present_after = false
	    };

	    egkr::render_target::attachment_configuration depth_attachment_configuration{
		.type = egkr::render_target::attachment_type::depth,
	        .source = egkr::render_target::attachment_source::default_source,
	        .load_op = egkr::render_target::load_operation::dont_care,
	        .store_op = egkr::render_target::store_operation::store,
	        .present_after = false
	    };

	    renderpass_configuration.target.attachments.push_back(colour_attachment_configuration);
	    renderpass_configuration.target.attachments.push_back(depth_attachment_configuration);

	    renderpass = renderpass::renderpass::create(renderpass_configuration);
	}
	
	std::string oit_shader_name = "Shader.Oit";
	auto shader = resource_system::load(oit_shader_name, resource::type::shader, nullptr);
	auto* properties = (shader::properties*)shader->data;
	shader_system::create_shader(*properties, renderpass.get());
	resource_system::unload(shader);

	oit_colour_shader = shader_system::get_shader(oit_shader_name);

	oit_shader_locations.projection = oit_colour_shader->get_uniform_index("projection");
	oit_shader_locations.view = oit_colour_shader->get_uniform_index("view");
	oit_shader_locations.model = oit_colour_shader->get_uniform_index("model");
	return true;
    }

    bool oit::destroy()
    {
	return true;
    }

    bool oit::execute(const frame_data& frame_data) const
    {
	engine::get()->get_renderer()->set_active_viewport(viewport);

	renderpass->begin(frame_data.render_target_index);

	if(!data.oit_geometries.empty())
	{
	    shader_system::use(oit_colour_shader->get_id());

	 //    if(oit_colour_shader->get_draw_index() == frame_data.draw_index && oit_colour_shader->get_frame_number() == frame_data.frame_number)
	 //    {
		// continue;
	 //    }

	    shader_system::set_uniform(oit_shader_locations.projection, &projection);
	    shader_system::set_uniform(oit_shader_locations.view, &view);

	    for(const auto& render_data : data.oit_geometries)
	    {
		auto mat = render_data.render_geometry->get_material();
		// bool needs_update = (mat->get_render_frame() != frame_data.frame_number || mat->get_draw_index() != frame_data.draw_index);

		// shader_system::apply_instance(needs_update);
		mat->set_render_frame(frame_data.frame_number);
		mat->set_draw_index(frame_data.draw_index);

		if (auto transform = render_data.transform.lock())
		{
		    auto model = transform->get_world();
		    shader_system::set_uniform(oit_shader_locations.model, &model);
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

	}
	renderpass->end();

	return true;

    }
}
