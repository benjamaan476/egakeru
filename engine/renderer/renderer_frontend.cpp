#include "renderer_frontend.h"

#include "systems/view_system.h"

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
	bool renderer_frontend::create(const platform::shared_ptr& platform)
	{
		renderer = std::make_unique<renderer_frontend>(platform);
		return true;
	}

	renderer_frontend::renderer_frontend(const platform::shared_ptr& platform)
		: platform_{platform}
	{
		const auto& size = platform->get_framebuffer_size();
		framebuffer_width_ = size.x;
		framebuffer_height_ = size.y;

		float4x4 view{ 1 };
		view = glm::translate(view, { 0.F, 0.F, 30.F });
		view = glm::inverse(view);
	}

	bool renderer_frontend::init()
	{
		renderer_backend::configuration configuration{};
		auto backen_init = backend_->init(configuration, window_attachment_count);

		return backen_init;
	}

	void renderer_frontend::shutdown()
	{
		backend_->shutdown();
	}

	void renderer_frontend::on_resize(uint32_t width, uint32_t height)
	{
		framebuffer_width_ = width;
		framebuffer_height_ = height;

		backend_->resize(width, height);
		view_system::on_window_resize(width, height);
	}

	void renderer_frontend::draw_frame(render_packet& packet) const
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

	texture* renderer_frontend::create_texture() const
	{
		return backend_->create_texture();
	}

	texture* renderer_frontend::create_texture(const texture::properties& properties, const uint8_t* data) const
	{
		return backend_->create_texture(properties, data);
	}

	void renderer_frontend::create_texture(const texture::properties& properties, const uint8_t* data, texture* out_texture) const
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

	render_target::render_target::shared_ptr renderer_frontend::create_render_target(const egkr::vector<render_target::attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height) const
	{
		return backend_->create_render_target(attachments, pass, width, height);
	}

	render_target::render_target::shared_ptr renderer_frontend::create_render_target(const egkr::vector<render_target::attachment_configuration>& attachments) const
	{
		return backend_->create_render_target(attachments);
	}

	renderpass::renderpass::shared_ptr renderer_frontend::create_renderpass(const renderpass::configuration& configuration) const
	{
		return backend_->create_renderpass(configuration);
	}

	texture_map::shared_ptr renderer_frontend::create_texture_map(const texture_map::properties& properties) const
	{
		return backend_->create_texture_map(properties);
	}

	renderbuffer::renderbuffer::shared_ptr renderer_frontend::create_renderbuffer(renderbuffer::type buffer_type, uint64_t size) const
	{
		return backend_->create_renderbuffer(buffer_type, size);
	}

	void renderer_frontend::set_viewport(const float4& rect) const
	{
		backend_->set_viewport(rect);
	}

	void renderer_frontend::reset_viewport() const
	{
		backend_->reset_viewport();
	}

	void renderer_frontend::set_scissor(const float4& rect) const
	{
		backend_->set_scissor(rect);
	}

	void renderer_frontend::reset_scissor() const
	{
		backend_->reset_scissor();
	}
}