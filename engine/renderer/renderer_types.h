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
		double delta_time{};
	};

	struct render_packet
	{
		double delta_time{};
		egkr::vector<geometry_render_data> world_geometry_data{};
		egkr::vector<geometry_render_data> ui_geometry_data{};
	};

	enum class builtin_renderpass
	{
		world,
		ui
	};

	class renderer_backend
	{
	public:
		using unique_ptr = std::unique_ptr<renderer_backend>;

		virtual ~renderer_backend() = default;
		virtual bool init() = 0;
		virtual void shutdown() = 0;
		virtual void resize(uint32_t width_, uint32_t height_) = 0;
		virtual bool begin_frame(double delta_time) = 0;
		virtual bool begin_renderpass(builtin_renderpass renderpass) = 0;
		virtual bool end_renderpass(builtin_renderpass renderpass) = 0;
		virtual void draw_geometry(const geometry_render_data& model) = 0;
		virtual void end_frame() = 0;

		virtual const void* get_context() const = 0;

		virtual void free_material(material* texture) = 0;

		virtual bool populate_texture(texture* texture, const texture_properties& properties, const uint8_t* data) = 0;
		virtual void free_texture(texture* texture) = 0;

		virtual bool populate_geometry(geometry* geometry, const geometry_properties& properties) = 0;
		virtual void free_geometry(geometry* geometry) = 0;

		virtual bool populate_shader(shader* shader, uint32_t renderpass_id, const egkr::vector<std::string>& stage_filenames, const egkr::vector<shader_stages>& shader_stages) = 0;
		virtual void free_shader(shader* shader) = 0;


		virtual bool use_shader(shader* shader) = 0;
		virtual bool bind_shader_globals(shader* shader) = 0;
		virtual bool bind_shader_instances(shader* shader, uint32_t instance_id) = 0;
		virtual bool apply_shader_globals(shader* shader) = 0;
		virtual bool apply_shader_instances(shader* shader) = 0;
		virtual uint32_t acquire_shader_isntance_resources(shader* shader) = 0;

		virtual bool set_uniform(shader* shader, const shader_uniform& uniform, const void* value) = 0;
	private:
		platform::shared_ptr platform_;
	};


}