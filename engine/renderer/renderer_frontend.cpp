#include "renderer_frontend.h"
#include "event.h"

#include "systems/texture_system.h"
#include "systems/geometry_system.h"
#include "systems/resource_system.h"
#include "systems/material_system.h"
#include "systems/shader_system.h"
#include "systems/camera_system.h"

#include "vulkan/renderer_vulkan.h"


#ifdef TRACY_MEMORY
void* operator new(std::size_t count)
{
	auto ptr = malloc(count);
	TracyAlloc(ptr, count);
	return ptr;
}
void operator delete(void* ptr) noexcept
{
	TracyFree(ptr);
	free(ptr);
}
#endif

namespace egkr
{
	void renderer_frontend::regenerate_render_targets()
	{
		for (auto i{ 0U }; i < window_attachment_count; ++i)
		{
			auto world = world_render_targets_[i].get();
			world->free(true);
			ui_render_targets_[i]->free(true);
			auto colour = backend_->get_window_attachment(i);
			auto depth = backend_->get_depth_attachment();

			world->populate({ colour, depth }, world_renderpass_, framebuffer_width_, framebuffer_height_);
			backend_->populate_render_target(ui_render_targets_[i].get(), {colour}, ui_renderpass_, framebuffer_width_, framebuffer_height_);
		}

	}

	renderer_frontend::unique_ptr renderer_frontend::create(backend_type type, const platform::shared_ptr& platform)
	{
		return std::make_unique<renderer_frontend>(type, platform);
	}

	renderer_frontend::renderer_frontend(backend_type type, const platform::shared_ptr& platform)
	{
		const auto& size = platform->get_framebuffer_size();
		framebuffer_width_ = size.x;
		framebuffer_height_ = size.y;
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
		egkr::vector<renderpass::configuration> renderpasses{};

		renderpass::configuration world{};
		world.name = "Renderpass.Builtin.World";
		world.previous_name = "";
		world.next_name = "ui";
		world.clear_colour = { 0.F, 0.F, 0.2F, 1.F };
		world.render_area = { 0, 0, framebuffer_width_, framebuffer_height_ };
		world.clear_flags = renderpass::clear_flags::all;
		renderpasses.push_back(world);

		renderpass::configuration ui{};
		ui.name = "Renderpass.Builtin.UI";
		ui.previous_name = "world";
		ui.next_name = "";
		ui.clear_flags = renderpass::clear_flags::none;
		ui.render_area = { 0, 0, framebuffer_width_, framebuffer_height_ };
		renderpasses.push_back(ui);

		renderer_backend_configuration configuration{};
		configuration.on_render_target_refresh_required = std::bind(&renderer_frontend::regenerate_render_targets, this);
		configuration.renderpass_configurations = renderpasses;

		auto backen_init = backend_->init(configuration, window_attachment_count);

		world_renderpass_ = backend_->get_renderpass("Renderpass.Builtin.World");
		ui_renderpass_ = backend_->get_renderpass("Renderpass.Builtin.UI");

		for (auto& render_target : world_render_targets_)
		{
			render_target = backend_->create_render_target();
		}

		for (auto& render_target : ui_render_targets_)
		{
			render_target = backend_->create_render_target();
		}


		regenerate_render_targets();

		auto resource = resource_system::load(BUILTIN_SHADER_NAME_MATERIAL, resource_type::shader);
		auto shader = (shader::properties*)resource->data;

		shader_system::create_shader(*shader);

		resource_system::unload(resource);

		material_shader_id = shader_system::get_shader_id(BUILTIN_SHADER_NAME_MATERIAL);

		auto ui_resource = resource_system::load(BUILTIN_SHADER_NAME_UI, resource_type::shader);
		auto ui_shader = (shader::properties*)ui_resource->data;

		shader_system::create_shader(*ui_shader);

		resource_system::unload(ui_resource);

		ui_shader_id = shader_system::get_shader_id(BUILTIN_SHADER_NAME_UI);

		event::register_event(event_code::render_mode, this, on_event);
		return backen_init;
	}

	void renderer_frontend::shutdown()
	{
		for (auto& target : world_render_targets_)
		{
			target->free(true);
		}
		world_render_targets_.clear();

		for (auto& target : ui_render_targets_)
		{
			target->free(true);
		}
		ui_render_targets_.clear();

		backend_->shutdown();
	}

	void renderer_frontend::on_resize(uint32_t width, uint32_t height)
	{
		framebuffer_width_ = width;
		framebuffer_height_ = height;
		world_renderpass_->get_render_area().z = width;
		world_renderpass_->get_render_area().w = height;
		ui_renderpass_->get_render_area().z = width;
		ui_renderpass_->get_render_area().w = height;


		world_projection_ = glm::perspective(glm::radians(45.0F), width / (float)height, near_clip_, far_clip_);
		ui_projection_ = glm::ortho(0.f, (float)width, (float)height, 0.F, -100.F, 100.F);
		backend_->resize(width, height);
	}

	void renderer_frontend::draw_frame(const render_packet& packet)
	{
		if (backend_->begin_frame())
		{
			auto attachment_index = backend_->get_window_index();

			if(world_renderpass_->begin(world_render_targets_[attachment_index].get()))
			{
				if (!shader_system::use(material_shader_id))
				{
					LOG_ERROR("Failed to use material shader");
					return;
				}

				if (!world_camera_)
				{
					world_camera_ = camera_system::get_default();
				}

				material_system::apply_global(material_shader_id, world_projection_, world_camera_->get_view(), ambient_colour_, world_camera_->get_position(), mode_);

				for (const auto& render_data : packet.world_geometry_data)
				{
					auto& material = render_data.geometry->get_material();
					bool needs_update = material->get_render_frame() != backend_->get_frame_number();
					{
						material_system::apply_instance(material, needs_update);
						material->set_render_frame(backend_->get_frame_number());
					}
						material_system::apply_local(material, render_data.model);

					backend_->draw_geometry(render_data);
				}

				if (!world_renderpass_->end())
				{
					LOG_ERROR("Failed to end world renderpass");
					return;
				}

				if(ui_renderpass_->begin(ui_render_targets_[attachment_index].get()))
				{
					if (!shader_system::use(ui_shader_id))
					{
						LOG_ERROR("Failed to use material shader");
						return;
					}

					material_system::apply_global(ui_shader_id, ui_projection_, ui_view_, {}, {}, 0);

					for (const auto& render_data : packet.ui_geometry_data)
					{
						auto material = render_data.geometry->get_material();
						bool needs_update = material->get_render_frame() != backend_->get_frame_number();

						material_system::apply_instance(material, needs_update);

						material_system::apply_local(material, render_data.model);
						backend_->draw_geometry(render_data);
					}

					if (!ui_renderpass_->end())
					{
						LOG_ERROR("Failed to end ui renderpass");
						return;
					}
				}
			}

			backend_->end_frame();
		}
	}

	void renderer_frontend::free_material(material* texture) const
	{
		return backend_->free_material(texture);
	}

	void renderer_frontend::acquire_texture_map(texture::texture_map* map) const
	{
		return backend_->acquire_texture_map(map);
	}

	void renderer_frontend::release_texture_map(texture::texture_map* map) const
	{
		return backend_->release_texture_map(map);
	}

	renderpass::renderpass* renderer_frontend::get_renderpass(std::string_view renderpass_name) const
	{
		return backend_->get_renderpass(renderpass_name);
	}
	void renderer_frontend::populate_render_target(render_target::render_target* render_target, const egkr::vector<texture::texture::shared_ptr>& attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height) const
	{
		backend_->populate_render_target(render_target, attachments, renderpass, width, height);
	}
	
	void renderer_frontend::free_render_target(render_target::render_target* render_target, bool free_internal_memory) const
	{
		render_target->free(free_internal_memory);
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