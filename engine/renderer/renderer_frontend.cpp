#include "renderer_frontend.h"
#include "event.h"

#include "systems/texture_system.h"
#include "systems/geometry_system.h"
#include "systems/resource_system.h"
#include "systems/material_system.h"
#include "systems/shader_system.h"
#include "systems/camera_system.h"
#include "systems/view_system.h"

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
			skybox_render_targets_[i]->free(true);

			auto colour = backend_->get_window_attachment(i);
			auto depth = backend_->get_depth_attachment();

			skybox_render_targets_[i]->populate({colour}, skybox_renderpass_, framebuffer_width_, framebuffer_height_);
			world->populate({ colour, depth }, world_renderpass_, framebuffer_width_, framebuffer_height_);
			ui_render_targets_[i]->populate({ colour }, ui_renderpass_, framebuffer_width_, framebuffer_height_);
		}
	}

	bool renderer_frontend::create(backend_type type, const platform::shared_ptr& platform)
	{
		renderer = std::make_unique<renderer_frontend>(type, platform);
		return true;
	}

	renderer_frontend::renderer_frontend(backend_type type, const platform::shared_ptr& platform)
	{
		const auto& size = platform->get_framebuffer_size();
		framebuffer_width_ = size.x;
		framebuffer_height_ = size.y;

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

		renderpass::configuration skybox{};
		skybox.name = "Renderpass.Builtin.Skybox";
		skybox.previous_name = "";
		skybox.next_name = "world-opaque";
		skybox.render_area = { 0, 0, framebuffer_width_, framebuffer_height_ };
		skybox.clear_flags = renderpass::clear_flags::colour;
		renderpasses.push_back(skybox);


		renderpass::configuration world{};
		world.name = "Renderpass.Builtin.World";
		world.previous_name = "skybox";
		world.next_name = "ui";
		world.clear_colour = { 0.F, 0.F, 0.2F, 1.F };
		world.render_area = { 0, 0, framebuffer_width_, framebuffer_height_ };
		world.clear_flags = renderpass::clear_flags::depth | renderpass::clear_flags::stencil;
		renderpasses.push_back(world);

		renderpass::configuration ui{};
		ui.name = "Renderpass.Builtin.UI";
		ui.previous_name = "world-opaque";
		ui.next_name = "";
		ui.clear_flags = renderpass::clear_flags::none;
		ui.render_area = { 0, 0, framebuffer_width_, framebuffer_height_ };
		renderpasses.push_back(ui);

		renderer_backend_configuration configuration{};
		configuration.on_render_target_refresh_required = std::bind(&renderer_frontend::regenerate_render_targets, this);
		configuration.renderpass_configurations = renderpasses;

		auto backen_init = backend_->init(configuration, window_attachment_count);

		skybox_renderpass_ = backend_->get_renderpass("Renderpass.Builtin.Skybox");
		world_renderpass_ = backend_->get_renderpass("Renderpass.Builtin.World");
		ui_renderpass_ = backend_->get_renderpass("Renderpass.Builtin.UI");

		for (auto& render_target : skybox_render_targets_)
		{
			render_target = backend_->create_render_target();
		}

		skybox_renderpass_->set_render_targets(skybox_render_targets_);

		for (auto& render_target : world_render_targets_)
		{
			render_target = backend_->create_render_target();
		}

		world_renderpass_->set_render_targets(world_render_targets_);

		for (auto& render_target : ui_render_targets_)
		{
			render_target = backend_->create_render_target();
		}

		ui_renderpass_->set_render_targets(ui_render_targets_);
		regenerate_render_targets();

		auto skybox_shader_resource = resource_system::load(BUILTIN_SHADER_NAME_SKYBOX, resource_type::shader, nullptr);
		shader_system::create_shader(*(shader::properties*)skybox_shader_resource->data);
		resource_system::unload(skybox_shader_resource);

		auto resource = resource_system::load(BUILTIN_SHADER_NAME_MATERIAL, resource_type::shader, nullptr);
		auto shader = (shader::properties*)resource->data;

		shader_system::create_shader(*shader);

		resource_system::unload(resource);


		auto ui_resource = resource_system::load(BUILTIN_SHADER_NAME_UI, resource_type::shader, nullptr);
		auto ui_shader = (shader::properties*)ui_resource->data;

		shader_system::create_shader(*ui_shader);

		resource_system::unload(ui_resource);

		skybox_shader_id = shader_system::get_shader_id(BUILTIN_SHADER_NAME_SKYBOX);
		material_shader_id = shader_system::get_shader_id(BUILTIN_SHADER_NAME_MATERIAL);
		ui_shader_id = shader_system::get_shader_id(BUILTIN_SHADER_NAME_UI);

		return backen_init;
	}

	void renderer_frontend::shutdown()
	{
		for (auto& target : skybox_render_targets_)
		{
			target->free(true);
		}
		skybox_render_targets_.clear();

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

		view_system::on_window_resize(width, height);

		world_renderpass_->get_render_area().z = width;
		world_renderpass_->get_render_area().w = height;
		ui_renderpass_->get_render_area().z = width;
		ui_renderpass_->get_render_area().w = height;

		backend_->resize(width, height);
	}

	void renderer_frontend::draw_frame(const render_packet& packet)
	{
		if (backend_->begin_frame())
		{
			auto attachment_index = backend_->get_window_index();
			for (auto& view : packet.render_views)
			{
				if (!view_system::on_render(view.render_view, &view, backend_->get_frame_number(), attachment_index))
				{
					LOG_ERROR("Failed to render view");
				}
			}

			backend_->end_frame();
		}
	}

	void renderer_frontend::free_material(material* texture) const
	{
		return backend_->free_material(texture);
	}

	texture::texture* renderer_frontend::create_texture() const
	{
		return backend_->create_texture();
	}

	texture::texture* renderer_frontend::create_texture(const texture::properties& properties, const uint8_t* data) const
	{
		return backend_->create_texture(properties, data);
	}

	void renderer_frontend::create_texture(const texture::properties& properties, const uint8_t* data, texture::texture* out_texture) const
	{
		backend_->create_texture(properties, data, out_texture);
	}

	shader::shader::shared_ptr renderer_frontend::create_shader(const shader::properties& properties) const
	{
		return backend_->create_shader(properties);
	}

	geometry::geometry::shared_ptr renderer_frontend::create_geometry(const geometry::properties& properties) const
	{
		return backend_->create_geometry(properties);
	}

	render_target::render_target::shared_ptr renderer_frontend::create_render_target() const
	{
		return backend_->create_render_target();
	}

	texture_map::texture_map::shared_ptr renderer_frontend::create_texture_map(const texture_map::properties& properties) const
	{
		return backend_->create_texture_map(properties);
	}

	renderbuffer::renderbuffer::shared_ptr renderer_frontend::create_renderbuffer(renderbuffer::type buffer_type, uint64_t size) const
	{
		return backend_->create_renderbuffer(buffer_type, size);
	}

	renderpass::renderpass* renderer_frontend::get_renderpass(std::string_view renderpass_name) const
	{
		return backend_->get_renderpass(renderpass_name);
	}

}