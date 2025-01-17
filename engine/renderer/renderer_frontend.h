#pragma once
#include "pch.h"
#include "renderer_types.h"
#include "renderpass.h"

#include "resources/material.h"
#include "resources/texture.h"
#include "resources/shader.h"
#include "viewport.h"

namespace egkr
{
    class renderer_frontend
    {
    public:
	using unique_ptr = std::unique_ptr<renderer_frontend>;
	API static bool create(backend_type type, const platform::shared_ptr& platform);

	renderer_frontend(backend_type type, const platform::shared_ptr& platform);

	API bool init();
	API void shutdown();
	API void tidy_up();
	API void on_resize(uint32_t width, uint32_t height);

	void free_material(material* texture) const;

	texture* create_texture() const;
	texture* create_texture(const texture::properties& properties, const uint8_t* data) const;
	void create_texture(const texture::properties& properties, const uint8_t* data, texture* out_texture) const;
	shader::shader::shared_ptr create_shader(const shader::properties& properties) const;
	geometry::geometry::shared_ptr create_geometry(const geometry::properties& properties) const;
	render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height) const;
	render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment_configuration>& attachments) const;
	renderpass::renderpass::shared_ptr create_renderpass(const renderpass::configuration& configuration) const;
	texture_map::shared_ptr create_texture_map(const texture_map::properties& properties) const;
	renderbuffer::renderbuffer::shared_ptr create_renderbuffer(renderbuffer::type buffer_type, uint64_t size) const;

	void set_scissor(const float4& rect) const;
	void reset_scissor() const;

	viewport* get_active_viewport() const { return active_viewport_; }

	void set_active_viewport(viewport* viewport);

	bool prepare_frame(frame_data& frame_data) const;
	bool begin(const frame_data& frame_data) const;
	void end(frame_data& frame_data) const;
	void present(const frame_data& frame_data) const;

	void set_winding(winding winding) const;

	const auto& get_backend() const { return backend_; }
    private:
	renderer_backend::unique_ptr backend_{};
	uint8_t window_attachment_count{};

	uint32_t framebuffer_width_{};
	uint32_t framebuffer_height_{};
	egkr::viewport* active_viewport_;
    };

    inline renderer_frontend::unique_ptr renderer{};
}
