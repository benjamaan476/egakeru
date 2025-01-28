#include "renderer_frontend.h"

namespace egkr
{
    bool renderer_frontend::create(backend_type type, const platform::shared_ptr& platform)
    {
	renderer = std::make_unique<renderer_frontend>(type, platform);
	return true;
    }

    renderer_frontend::renderer_frontend(backend_type type, const platform::shared_ptr& platform)
    {
	const auto& size = platform->get_framebuffer_size();
	framebuffer_width_ = (uint32_t)size.x;
	framebuffer_height_ = (uint32_t)size.y;

	float4x4 view{1};
	view = glm::translate(view, {0.F, 0.F, 30.F});
	view = glm::inverse(view);
	switch (type)
	{
	case backend_type::vulkan:
	    backend_ = renderer_backend::create(platform); // renderer_vulkan::create(platform);
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
	renderer_backend::configuration configuration{};
	auto backen_init = backend_->init(configuration, window_attachment_count);

	return backen_init;
    }

    void renderer_frontend::shutdown() { backend_->shutdown(); }

    API void renderer_frontend::tidy_up() { return backend_->tidy_up(); }

    void renderer_frontend::on_resize(uint32_t width, uint32_t height)
    {
	framebuffer_width_ = width;
	framebuffer_height_ = height;

	backend_->resize(width, height);
    }

    void renderer_frontend::free_material(material* texture) const { backend_->free_material(texture); }

    texture::shared_ptr renderer_frontend::create_texture() const { return backend_->create_texture(); }

    texture::shared_ptr renderer_frontend::create_texture(const texture::properties& properties, const uint8_t* data) const { return backend_->create_texture(properties, data); }

    void renderer_frontend::create_texture(const texture::properties& properties, const uint8_t* data, texture* out_texture) const { backend_->create_texture(properties, data, out_texture); }

    shader::shader::shared_ptr renderer_frontend::create_shader(const shader::properties& properties) const { return backend_->create_shader(properties); }

    geometry::geometry::shared_ptr renderer_frontend::create_geometry(const geometry::properties& properties) const { return backend_->create_geometry(properties); }

    render_target::render_target::shared_ptr renderer_frontend::create_render_target(
        const egkr::vector<render_target::attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height) const
    {
	return backend_->create_render_target(attachments, pass, width, height);
    }

    render_target::render_target::shared_ptr renderer_frontend::create_render_target(const egkr::vector<render_target::attachment_configuration>& attachments) const
    {
	return backend_->create_render_target(attachments);
    }

    renderpass::renderpass::shared_ptr renderer_frontend::create_renderpass(const renderpass::configuration& configuration) const { return backend_->create_renderpass(configuration); }

    texture_map::shared_ptr renderer_frontend::create_texture_map(const texture_map::properties& properties) const { return backend_->create_texture_map(properties); }

    renderbuffer::renderbuffer::shared_ptr renderer_frontend::create_renderbuffer(renderbuffer::type buffer_type, uint64_t size) const { return backend_->create_renderbuffer(buffer_type, size); }

    void renderer_frontend::set_scissor(const float4& rect) const { backend_->set_scissor(rect); }

    void renderer_frontend::reset_scissor() const { backend_->reset_scissor(); }

    void renderer_frontend::set_active_viewport(viewport* viewport)
    {
	active_viewport_ = viewport;
	float4 viewport_rect{viewport->viewport_rect.x, viewport->viewport_rect.y + viewport->viewport_rect.w, viewport->viewport_rect.z, -viewport->viewport_rect.w};
	float4 scissor_rect{viewport->viewport_rect.x, viewport->viewport_rect.y, viewport->viewport_rect.z, viewport->viewport_rect.w};
	backend_->set_viewport(viewport_rect);
	backend_->set_scissor(scissor_rect);
    }

    bool renderer_frontend::prepare_frame(frame_data& frame_data) const { return backend_->prepare_frame(frame_data); }

    bool renderer_frontend::begin(const frame_data& frame_data) const { return backend_->begin(frame_data); }

    void renderer_frontend::end(frame_data& frame_data) const { backend_->end(frame_data); }

    void renderer_frontend::present(const frame_data& frame_data) const { backend_->present(frame_data); }

    void renderer_frontend::set_winding(winding winding) const { backend_->set_winding(winding); }
}
