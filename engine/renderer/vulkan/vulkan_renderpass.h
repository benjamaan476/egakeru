#pragma once

#include "pch.h"
#include "command_buffer.h"
#include "renderer/renderer_types.h"

namespace egkr
{
	struct vulkan_context;
	enum class renderpass_state
	{
		ready,
		recording,
		in_render_pass,
		recording_ended,
		submitted,
		not_allocated
	};



	struct renderpass_properties
	{
		uint4 render_extent{};

		renderpass_clear_flags clear_flags{};
		float4 clear_colour{};
		float_t depth{};
		uint32_t stencil{};

		bool has_previous_pass{};
		bool has_next_pass{};

	};

	class vulkan_renderpass : std::enable_shared_from_this<vulkan_renderpass>
	{
	public:
		using shared_ptr = std::shared_ptr<vulkan_renderpass>;
		static shared_ptr create(const vulkan_context* context, const renderpass_properties& properties);

		vulkan_renderpass(const vulkan_context* context, const renderpass_properties& properties);
		~vulkan_renderpass();

		void destroy();

		void begin(command_buffer& command_buffer, vk::Framebuffer framebuffer);
		static void end(command_buffer& command_buffer);

		void set_extent(uint4 extent);

		const auto& get_handle() { return renderpass_; }

		const vulkan_context* context_{};
		uint4 render_extent_{};
		vk::RenderPass renderpass_{};
		renderpass_clear_flags clear_flags_{};
		float4 clear_colour_{};
		float_t depth_{};
		uint32_t stencil_{};

		bool has_previous_pass_{};
		bool has_next_pass_{};

		renderpass_state state_{};
	};
}
