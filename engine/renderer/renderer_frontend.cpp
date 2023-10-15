#include "renderer_frontend.h"
#include "event.h"

#include "systems/texture_system.h"
#include "systems/geometry_system.h"
#include "systems/resource_system.h"
#include "systems/material_system.h"
#include "systems/shader_system.h"

#include "vulkan/renderer_vulkan.h"

namespace egkr
{
	renderer_frontend::unique_ptr renderer_frontend::create(backend_type type, const platform::shared_ptr& platform)
	{
		return std::make_unique<renderer_frontend>(type, platform);
	}

	renderer_frontend::renderer_frontend(backend_type type, const platform::shared_ptr& platform)
	{
		const auto& size = platform->get_framebuffer_size();
		world_projection_ = glm::perspective(glm::radians(45.0F), size.x / (float)size.y, 0.1F, 1000.F);
		ui_projection_ = glm::ortho(0.F, (float)size.x, (float)size.y, 0.F, -100.F, 100.F);

		float4x4 view{ 1 };
		view = glm::translate(view, { 0.F, 0.F, 30.F });
		view = glm::inverse(view);
		switch (type)
		{
		case backend_type::vulkan:
			backend_ = renderer_vulkan::create(platform);
			break;
		case backend_type::opengl:
		case backend_type::directx:
		default:
			LOG_ERROR("Unsupported renderer backend chosen");
			break;
		}

	}

	bool renderer_frontend::init()
	{
		auto backen_init = backend_->init();

		auto resource = resource_system::load(BUILTIN_SHADER_NAME_MATERIAL, resource_type::shader);
		auto shader = (shader_properties*)resource->data;

		shader_system::create_shader(*shader);

		resource_system::unload(resource);

		material_shader_id = shader_system::get_shader_id(BUILTIN_SHADER_NAME_MATERIAL);

		auto ui_resource = resource_system::load(BUILTIN_SHADER_NAME_UI, resource_type::shader);
		auto ui_shader = (shader_properties*)ui_resource->data;

		shader_system::create_shader(*ui_shader);

		resource_system::unload(ui_resource);

		ui_shader_id = shader_system::get_shader_id(BUILTIN_SHADER_NAME_UI);

		event::register_event(event_code::render_mode, this, on_event);
		return backen_init;
	}

	void renderer_frontend::shutdown()
	{
		backend_->shutdown();
	}

	void renderer_frontend::on_resize(uint32_t width, uint32_t height)
	{
		world_projection_ = glm::perspective(glm::radians(45.0F), width / (float)height, near_clip_, far_clip_);
		ui_projection_ = glm::ortho(0.f, (float)width, (float)height, 0.F, -100.F, 100.F);
		backend_->resize(width, height);
	}

	void renderer_frontend::draw_frame(const render_packet& packet)
	{
		if (backend_->begin_frame(packet.delta_time))
		{
			if (backend_->begin_renderpass(builtin_renderpass::world))
			{
				if (!shader_system::use(material_shader_id))
				{
					LOG_ERROR("Failed to use material shader");
					return;
				}

				material_system::apply_global(material_shader_id, world_projection_, world_view_, ambient_colour_, camera_position_, mode_);

				for (const auto& render_data : packet.world_geometry_data)
				{
					material_system::apply_instance(render_data.geometry->get_material());

					material_system::apply_local(render_data.geometry->get_material(), render_data.model);
					backend_->draw_geometry(render_data);
				}

				if (!backend_->end_renderpass(builtin_renderpass::world))
				{
					LOG_ERROR("Failed to end world renderpass");
					return;
				}

				if (backend_->begin_renderpass(builtin_renderpass::ui))
				{
					if (!shader_system::use(ui_shader_id))
					{
						LOG_ERROR("Failed to use material shader");
						return;
					}

					material_system::apply_global(ui_shader_id, ui_projection_, ui_view_, {}, {}, 0);

					for (const auto& render_data : packet.ui_geometry_data)
					{
						material_system::apply_instance(render_data.geometry->get_material());

						material_system::apply_local(render_data.geometry->get_material(), render_data.model);
						backend_->draw_geometry(render_data);
					}

					if (!backend_->end_renderpass(builtin_renderpass::ui))
					{
						LOG_ERROR("Failed to end ui renderpass");
						return;
					}
				}
			}

			backend_->end_frame();
		}
	}

	void renderer_frontend::set_view(const float4x4& view, const float3& camera_position)
	{
		world_view_ = view;
		camera_position_ = camera_position;
	}

	void renderer_frontend::free_material(material* texture) const
	{
		return backend_->free_material(texture);
	}

	bool renderer_frontend::populate_texture(texture* texture, const texture_properties& properties, const uint8_t* data) const
	{
		return backend_->populate_texture(texture, properties, data);
	}

	void renderer_frontend::free_texture(texture* texture) const
	{
		backend_->free_texture(texture);
	}

	bool renderer_frontend::populate_geometry(geometry* geometry, const geometry_properties& properties) const
	{
		return backend_->populate_geometry(geometry, properties);
	}

	void renderer_frontend::free_geometry(geometry* geometry) const
	{
		backend_->free_geometry(geometry);
	}

	bool renderer_frontend::populate_shader(shader* shader, uint32_t renderpass_id, const egkr::vector<std::string>& stage_filenames, const egkr::vector<shader_stages>& shader_stages) const
	{
		return backend_->populate_shader(shader, renderpass_id, stage_filenames, shader_stages);
	}
	void renderer_frontend::free_shader(shader* shader) const
	{
		backend_->free_shader(shader);
	}

	bool renderer_frontend::use_shader(shader* shader) const
	{
		return backend_->use_shader(shader);
	}

	bool renderer_frontend::bind_shader_globals(shader* shader) const
	{
		return backend_->bind_shader_globals(shader);
	}

	bool renderer_frontend::bind_shader_instances(shader* shader, uint32_t instance_id) const
	{
		return backend_->bind_shader_instances(shader, instance_id);
	}

	bool renderer_frontend::apply_shader_globals(shader* shader) const
	{
		return backend_->apply_shader_globals(shader);
	}

	bool renderer_frontend::apply_shader_instances(shader* shader) const
	{
		return backend_->apply_shader_instances(shader);
	}

	uint32_t renderer_frontend::acquire_shader_isntance_resources(shader* shader) const
	{
		return backend_->acquire_shader_isntance_resources(shader);
	}

	bool renderer_frontend::set_uniform(shader* shader, const shader_uniform& uniform, const void* value) const
	{
		return backend_->set_uniform(shader, uniform, value);
	}

	builtin_renderpass renderer_frontend::get_renderpass_id(std::string_view renderpass_name) const
	{
		if (renderpass_name == "Renderpass.Builtin.World")
		{
			return builtin_renderpass::world;
		}
		else if (renderpass_name == "Renderpass.Builtin.UI")
		{
			return builtin_renderpass::ui;
		}
		else
		{
			LOG_ERROR("Unknown renderpass name requested: {}", renderpass_name.data());
			return builtin_renderpass::world;
		}
	}

	bool renderer_frontend::on_event(event_code code, void* /*sender*/, void* listener, const event_context& context)
	{
		if (code == event_code::render_mode)
		{
			const auto& context_array = std::get<std::array<uint32_t, 4>>(context);
			const auto& mode = context_array[0];

			((renderer_frontend*)listener)->mode_ = mode;
		}
		return false;
	}
}