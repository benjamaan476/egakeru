#pragma once
#include "pch.h"

#include "resources/geometry.h"
#include "resources/shader.h"

#include "platform/platform.h"

#include "vertex_types.h"
#include "renderpass.h"
#include "render_target.h"

namespace egkr
{
	static std::string BUILTIN_SHADER_NAME_MATERIAL{"Shader.Builtin.Material"};
	static std::string BUILTIN_SHADER_NAME_UI{"Shader.Builtin.UI"};

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
		egkr::vector<geometry::render_data> world_geometry_data{};
		egkr::vector<geometry::render_data> ui_geometry_data{};
	};

	struct renderer_backend_configuration
	{
		std::string application_name{};
		egkr::vector<renderpass::configuration> renderpass_configurations{};

		std::function<void(void)> on_render_target_refresh_required{};
	};

	class renderer_backend
	{
	public:
		using unique_ptr = std::unique_ptr<renderer_backend>;

		virtual ~renderer_backend() = default;
		virtual bool init(const renderer_backend_configuration& configuration, uint8_t& out_window_attachment_count) = 0;
		virtual void shutdown() = 0;
		virtual void resize(uint32_t width_, uint32_t height_) = 0;
		virtual bool begin_frame() = 0;
		virtual void draw_geometry(const geometry::render_data& model) = 0;
		virtual void end_frame() = 0;

		virtual const void* get_context() const = 0;

		virtual void free_material(material* texture) const = 0;

		virtual texture::texture::shared_ptr create_texture(const texture::properties& properties, const uint8_t* data) const = 0;
		virtual shader::shader::shared_ptr create_shader(const shader::properties& properties) const = 0;
		virtual geometry::geometry::shared_ptr create_geometry(const geometry::properties& properties) const = 0;
		virtual render_target::render_target::shared_ptr create_render_target() const = 0;

		virtual void populate_render_target(render_target::render_target* render_target, egkr::vector<texture::texture::shared_ptr> attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height) = 0;

		virtual void acquire_texture_map(texture::texture_map* map) const = 0;
		virtual void release_texture_map(texture::texture_map* map) const = 0;

		virtual texture::texture::shared_ptr get_window_attachment(uint8_t index) = 0;
		virtual texture::texture::shared_ptr get_depth_attachment() = 0;
		virtual uint8_t get_window_index() = 0;

		virtual renderpass::renderpass* get_renderpass(std::string_view name) const = 0;

		uint32_t get_frame_number() const { return frame_number_; }
	protected:
		void new_frame() { ++frame_number_; }
	private:
		platform::shared_ptr platform_;
		uint32_t frame_number_{ invalid_32_id };
	};


}