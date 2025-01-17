#include "skybox_pass.h"
#include <systems/resource_system.h>
#include <systems/shader_system.h>

#include <renderer/renderer_frontend.h>

namespace egkr::pass
{
	bool skybox::init()
	{
		egkr::renderpass::configuration skybox_renderpass_configuration
		{
			.name = "Renderpass.Skybox",
			.clear_colour = {0, 0, 0.2F, 1.F},
			.pass_clear_flags = egkr::renderpass::clear_flags::colour,
			.depth = 1.F,
			.stencil = 0
		};

		egkr::render_target::attachment_configuration skybox_attachment_configration
		{
			.type = egkr::render_target::attachment_type::colour,
			.source = egkr::render_target::attachment_source::default_source,
			.load_op = egkr::render_target::load_operation::dont_care,
			.store_op = egkr::render_target::store_operation::store,
			.present_after = false
		};

		skybox_renderpass_configuration.target.attachments.push_back(skybox_attachment_configration);

		pass->create(skybox_renderpass_configuration);

			const std::string shader_name = "Shader.Skybox";
			auto skybox_shader_resource = resource_system::load(shader_name, resource::type::shader, nullptr);
			auto skybox_shader = (shader::properties*)skybox_shader_resource->data;
			shader_system::create_shader(*skybox_shader, pass.get());
			resource_system::unload(skybox_shader_resource);

			shader = shader_system::get_shader(shader_name);

			shader_locations.projection = shader->get_uniform_index("projection");
			shader_locations.view = shader->get_uniform_index("view");
			shader_locations.cubemap = shader->get_uniform_index("cube_texture");
		return true;
	}

	bool skybox::execute(const frame_data& frame_data) const
	{
		renderer->set_active_viewport(viewport);
		pass->begin(pass->get_render_target(frame_data.render_target_index).get());
		return false;
	}
}