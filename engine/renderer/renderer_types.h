#pragma once
#include "pch.h"

#include "platform/platform.h"

#include "resources/texture.h"

namespace egkr
{
	enum class backend_type
	{
		vulkan,
		opengl,
		directx
	};

	struct render_packet
	{
		std::chrono::milliseconds delta_time{};
	};

	struct global_uniform_buffer
	{
		float4x4 projection{};
		float4x4 view{};
		float4x4 reserved0{}; // make this 256 bytes in size
		float4x4 reserve1{};
	};

	struct object_uniform_object
	{
		float4 diffuse_colour{};
		float4 pad0{};
		float4 pad1{};
		float4 pad2{};
	};

	struct geometry_render_data
	{
		uint32_t object_id{};
		float4x4 model{};
		std::array<texture::shared_ptr, 16> textures{};
	};

	class renderer_backend
	{
	public:
		using unique_ptr = std::unique_ptr<renderer_backend>;

		virtual ~renderer_backend() = default;
		virtual bool init() = 0;
		virtual void shutdown() = 0;
		virtual void resize(uint32_t width_, uint32_t height_) = 0;
		virtual bool begin_frame(std::chrono::milliseconds delta_time) = 0;
		virtual void update_global_state(const float4x4& projection, const float4x4& view, const float3& view_postion, const float4& ambient_colour, int32_t mode) = 0;
		virtual void update(const geometry_render_data& model) = 0;
		virtual void end_frame() = 0;
	private:
		platform::shared_ptr platform_;
	};


}