#pragma once
#include "pch.h"

#include "resources/geometry.h"
#include "resources/shader.h"

#include "platform/platform.h"

#include "vertex_types.h"
#include "renderpass.h"
#include "render_target.h"
#include "renderbuffer.h"
#include <utility>

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

    enum class winding
    {
	counter_clockwise,
	clockwise
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
	    std::string application_name;
	    flags backend_flags;
	};

	using unique_ptr = std::unique_ptr<renderer_backend>;
	virtual ~renderer_backend() = default;

	static unique_ptr create(const platform::shared_ptr& platform);
	virtual bool init(const configuration& configuration, uint8_t& out_window_attachment_count) = 0;
	virtual void shutdown() = 0;
	virtual void tidy_up() = 0;
	virtual void resize(uint32_t width_, uint32_t height_) = 0;

	virtual bool prepare_frame(frame_data& frame_data) = 0;
	virtual bool begin(const frame_data& frame_data) = 0;
	virtual void end(frame_data& frame_data) = 0;
	virtual void present(const frame_data& frame_data) = 0;

	virtual void free_material(material* texture) const = 0;

	[[nodiscard]] virtual texture::shared_ptr create_texture() const = 0;
	virtual texture::shared_ptr create_texture(const texture::properties& properties, const uint8_t* data) const = 0;
	virtual void create_texture(const texture::properties& properties, const uint8_t* data, texture* out_texture) const = 0;
	[[nodiscard]] virtual shader::shader::shared_ptr create_shader(const shader::properties& properties) const = 0;
	[[nodiscard]] virtual geometry::geometry::shared_ptr create_geometry(const geometry::properties& properties) const = 0;
	virtual render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height) const
	    = 0;
	[[nodiscard]] virtual render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment_configuration>& attachments) const = 0;
	[[nodiscard]] virtual renderpass::renderpass::shared_ptr create_renderpass(const renderpass::configuration& configuration) const = 0;
	[[nodiscard]] virtual texture_map::shared_ptr create_texture_map(const texture_map::properties& properties) const = 0;
	[[nodiscard]] virtual renderbuffer::renderbuffer::shared_ptr create_renderbuffer(renderbuffer::type buffer_type, uint64_t size) const = 0;

	virtual void set_viewport(const float4& rect) const = 0;
	virtual void set_scissor(const float4& rect) const = 0;
	virtual void reset_scissor() const = 0;
	virtual void set_winding(winding winding) const = 0;

	[[nodiscard]] virtual uint32_t get_window_attachment_count() const = 0;
	[[nodiscard]] virtual texture::shared_ptr get_window_attachment(uint8_t index) const = 0;
	[[nodiscard]] virtual texture::shared_ptr get_depth_attachment(uint8_t index) const = 0;
	[[nodiscard]] virtual uint8_t get_window_index() const = 0;

	[[nodiscard]] uint64_t get_frame_number() const { return frame_number_; }
	[[nodiscard]] uint64_t get_draw_index() const { return draw_index_; }

	[[nodiscard]] virtual bool is_multithreaded() const = 0;
    protected:
	void new_frame() { ++frame_number_; }
	uint64_t frame_number_{0};
	uint64_t draw_index_{invalid_64_id};
    };
}
