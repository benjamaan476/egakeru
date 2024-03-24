#pragma once
#include "pch.h"

#include "resources/geometry.h"
#include "resources/shader.h"

#include "platform/platform.h"

#include "vertex_types.h"
#include "render_view.h"
#include "renderpass.h"
#include "render_target.h"
#include "renderbuffer.h"

namespace egkr
{
	enum class backend_type
	{
		vulkan,
		opengl,
		directx
	};

	struct material_shader_uniform_buffer_object
	{
		float4x4 projection{};
		float4x4 view{};
		float4x4 reserved0{}; // make this 256 bytes in size
		float4x4 reserve1{};
	};

	struct material_instance_uniform_buffer_object
	{
		float4 diffuse_colour{};
		float4 pad0{};
		float4 pad1{};
		float4 pad2{};
	};

	struct render_packet
	{
		egkr::vector<render_view::render_view_packet> render_views{};
	};

	class renderer_backend
	{
	public:

		enum flags : uint8_t
		{
			vsync = 0x01,
			power_saving = 0x02
		};

		struct configuration
		{
			std::string application_name{};
			flags flags;
		};

		using unique_ptr = std::unique_ptr<renderer_backend>;
		virtual ~renderer_backend() = default;
		virtual bool init(const configuration& configuration, uint8_t& out_window_attachment_count) = 0;
		virtual void shutdown() = 0;
		virtual void resize(uint32_t width_, uint32_t height_) = 0;
		virtual bool begin_frame() = 0;
		virtual void end_frame() = 0;

		virtual void free_material(material* texture) const = 0;

		virtual texture::texture* create_texture() const = 0;
		virtual texture::texture* create_texture(const texture::properties& properties, const uint8_t* data) const = 0;
		virtual void create_texture(const texture::properties& properties, const uint8_t* data, texture::texture* out_texture) const = 0;
		virtual shader::shader::shared_ptr create_shader(const shader::properties& properties) const = 0;
		virtual geometry::geometry::shared_ptr create_geometry(const geometry::properties& properties) const = 0;
		virtual render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height) const = 0;
		virtual render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment_configuration>& attachments) const = 0;
		virtual renderpass::renderpass::shared_ptr create_renderpass(const renderpass::configuration& configuration) const = 0;
		virtual texture_map::texture_map::shared_ptr create_texture_map(const texture_map::properties& properties) const = 0;
		virtual renderbuffer::renderbuffer::shared_ptr create_renderbuffer(renderbuffer::type buffer_type, uint64_t size) const = 0;

		virtual void set_viewport(const float4& rect) const = 0;
		virtual void reset_viewport() const = 0;
		virtual void set_scissor(const float4& rect) const = 0;
		virtual void reset_scissor() const = 0;

		virtual texture::texture* get_window_attachment(uint8_t index) const = 0;
		virtual texture::texture* get_depth_attachment(uint8_t index) const = 0;
		virtual uint8_t get_window_index() const = 0;

		uint32_t get_frame_number() const { return frame_number_; }

		virtual bool is_multithreaded() const = 0;
	protected:
		void new_frame() { ++frame_number_; }
	private:
		platform::shared_ptr platform_;
		uint32_t frame_number_{ invalid_32_id };
	};


}