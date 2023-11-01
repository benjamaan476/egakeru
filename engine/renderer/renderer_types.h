#pragma once
#include "pch.h"

#include "resources/geometry.h"
#include "resources/shader.h"

#include "platform/platform.h"

#include "vertex_types.h"

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

	struct geometry_render_data
	{
		geometry::shared_ptr geometry{};
		float4x4 model{};
	};

	struct render_packet
	{
		egkr::vector<geometry_render_data> world_geometry_data{};
		egkr::vector<geometry_render_data> ui_geometry_data{};
	};

	enum renderpass_clear_flags
	{
		none,
		colour = 0x01,
		depth = 0x02,
		stencil = 0x04,
		all = colour | depth | stencil
	};
	ENUM_CLASS_OPERATORS(renderpass_clear_flags)

	struct render_target
	{
		using shared_ptr = std::shared_ptr<render_target>;
		bool sync_to_window_size{};
		egkr::vector<texture::shared_ptr> attachments{};

		void* internal_framebuffer;
};

	struct renderpass
	{
		using shared_ptr = std::shared_ptr<renderpass>;
		uint16_t id{};
		float4 render_area{};
		float4 clear_colour{};
		renderpass_clear_flags clear_flags{};
		egkr::vector<render_target::shared_ptr> render_targets{3};

		void* internal_data{};
	};

	struct renderpass_configuration
	{
		std::string name{};
		std::string previous_name{};
		std::string next_name{};
		float4 render_area{};
		float4 clear_colour{};
		renderpass_clear_flags clear_flags{};
	};

	struct renderer_backend_configuration
	{
		std::string application_name{};
		egkr::vector<renderpass_configuration> renderpass_configurations{};

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
		virtual bool begin_renderpass(renderpass* renderpass, render_target* render_target) = 0;
		virtual bool end_renderpass(renderpass* renderpass) = 0;
		virtual void draw_geometry(const geometry_render_data& model) = 0;
		virtual void end_frame() = 0;

		virtual const void* get_context() const = 0;

		virtual void free_material(material* texture) = 0;

		virtual bool populate_texture(texture* texture, const texture_properties& properties, const uint8_t* data) = 0;
		virtual bool populate_writeable_texture(texture* texture) = 0;
		virtual bool resize_texture(texture* texture, uint32_t width, uint32_t height) = 0;
		virtual bool texture_write_data(texture* texture, uint64_t offset, uint32_t size, const uint8_t* data) = 0;
		virtual void free_texture(texture* texture) = 0;
		virtual void populate_render_target(render_target* render_target, egkr::vector<texture::shared_ptr> attachments, renderpass* renderpass, uint32_t width, uint32_t height) = 0;
		virtual void free_render_target(render_target* render_target, bool free_internal_memory) = 0;
		virtual void populate_renderpass(renderpass* renderpass, float depth, uint32_t stencil, bool has_previous, bool has_next) = 0;
		virtual void free_renderpass(renderpass* renderpass) = 0;

		virtual bool populate_geometry(geometry* geometry, const geometry_properties& properties) = 0;
		virtual void free_geometry(geometry* geometry) = 0;

		virtual bool populate_shader(shader* shader, renderpass* renderpass, const egkr::vector<std::string>& stage_filenames, const egkr::vector<shader_stages>& shader_stages) = 0;
		virtual void free_shader(shader* shader) = 0;


		virtual bool use_shader(shader* shader) = 0;
		virtual bool bind_shader_globals(shader* shader) = 0;
		virtual bool bind_shader_instances(shader* shader, uint32_t instance_id) = 0;
		virtual bool apply_shader_globals(shader* shader) = 0;
		virtual bool apply_shader_instances(shader* shader, bool needs_update) = 0;
		virtual uint32_t acquire_shader_isntance_resources(shader* shader, const egkr::vector<texture_map*>& texture_maps) = 0;
		virtual void acquire_texture_map(texture_map* map) = 0;
		virtual void release_texture_map(texture_map* map) = 0;

		virtual bool set_uniform(shader* shader, const shader_uniform& uniform, const void* value) = 0;
		virtual texture::shared_ptr get_window_attachment(uint8_t index) = 0;
		virtual texture::shared_ptr get_depth_attachment() = 0;
		virtual uint8_t get_window_index() = 0;

		virtual renderpass* get_renderpass(std::string_view name) = 0;

		uint32_t get_frame_number() const { return frame_number_; }
	protected:
		void new_frame() { ++frame_number_; }
	private:
		platform::shared_ptr platform_;
		uint32_t frame_number_{ invalid_32_id };
	};


}