#include "skybox_pass.h"
#include <systems/resource_system.h>
#include <systems/shader_system.h>

#include "engine/engine.h"
#include <renderer/renderpass.h>

namespace egkr::pass
{
	skybox* skybox::create()
	{
		auto pass = new skybox();
		pass->init();
		return pass;
	}

	bool skybox::init()
	{
		name = "skybox";
		egkr::renderpass::configuration skybox_renderpass_configuration{ .name = name, .clear_colour = {0, 0, 0.2F, 1.F}, .pass_clear_flags = egkr::renderpass::clear_flags::colour, .depth = 1.F, .stencil = 0 };

		egkr::render_target::attachment_configuration skybox_attachment_configration{ .type = egkr::render_target::attachment_type::colour,
			.source = egkr::render_target::attachment_source::default_source,
			.load_op = egkr::render_target::load_operation::dont_care,
			.store_op = egkr::render_target::store_operation::store,
			.present_after = false };

		skybox_renderpass_configuration.target.attachments.push_back(skybox_attachment_configration);

		renderpass = egkr::renderpass::renderpass::create(skybox_renderpass_configuration);

		const std::string shader_name = "Shader.Skybox";
		auto skybox_shader_resource = resource_system::load(shader_name, resource::type::shader, nullptr);
		auto skybox_shader = (shader::properties*)skybox_shader_resource->data;
		shader_system::create_shader(*skybox_shader, renderpass.get());
		resource_system::unload(skybox_shader_resource);

		shader = shader_system::get_shader(shader_name);

		shader_locations.projection = shader->get_uniform_index("projection");
		shader_locations.view = shader->get_uniform_index("view");
		shader_locations.cubemap = shader->get_uniform_index("cube_texture");
		return true;
	}

	bool skybox::execute(const frame_data& frame_data) const
	{
		engine::get()->get_renderer()->set_active_viewport(viewport);
		renderpass->begin(frame_data.render_target_index);
		if (auto& skybox = data.skybox_data)
		{
			shader_system::use(shader->get_id());
			//Reset view position so that skybox doesn't move
			float4x4 skybox_view = this->view;
			skybox_view[3][0] = 0.F;
			skybox_view[3][1] = 0.F;
			skybox_view[3][2] = 0.F;

			shader->bind_globals();
			shader_system::set_uniform(shader_locations.projection, &projection);
			shader_system::set_uniform(shader_locations.view, &skybox_view);

			bool needs_update = (skybox->get_frame_number() != frame_data.frame_number) || (skybox->get_draw_index() != frame_data.draw_index);
			shader_system::apply_global(needs_update);

			shader_system::bind_instance(skybox->get_instance_id());
			shader_system::set_uniform(shader_locations.cubemap, &skybox->get_texture_map());

			shader_system::apply_instance(needs_update);
			skybox->set_frame_number(frame_data.frame_number);
			skybox->set_draw_index(frame_data.draw_index);

			skybox->draw();
		}
		renderpass->end();
		return true;
	}

	bool skybox::destroy()
	{
		shader->free();
		shader.reset();

		renderpass->free();
		renderpass.reset();

		return true;
	}
}
