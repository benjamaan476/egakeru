#pragma once
#include "pch.h"

#include "platform/platform.h"

#include "resources/geometry.h"

#include "vertex_types.h"

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
		virtual void update_world_state(const float4x4& projection, const float4x4& view, const float3& view_postion, const float4& ambient_colour, int32_t mode) = 0;
		virtual void update_ui_state(const float4x4& projection, const float4x4& view, const float3& view_postion, const float4& ambient_colour, int32_t mode) = 0;
		virtual void draw_geometry(const geometry_render_data& model) = 0;
		virtual void end_frame() = 0;

		virtual const void* get_context() const = 0;

	private:
		platform::shared_ptr platform_;
	};


}