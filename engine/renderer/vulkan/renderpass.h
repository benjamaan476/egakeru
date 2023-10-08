#pragma once

#include "pch.h"
#include "command_buffer.h"

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

	enum renderpass_clear_flags
	{
		none,
		colour = 0x01,
		depth = 0x02,
		stencil = 0x04
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

	class renderpass : std::enable_shared_from_this<renderpass>
	{
	public:
		using shared_ptr = std::shared_ptr<renderpass>;
		static shared_ptr create(const vulkan_context* context, const renderpass_properties& properties);

		renderpass(const vulkan_context* context, const renderpass_properties& properties);
		~renderpass();

		void destroy();

		void begin(command_buffer& command_buffer, vk::Framebuffer framebuffer);
		static void end(command_buffer& command_buffer);

		void set_extent(uint4 extent);

		const auto& get_handle() { return renderpass_; }
	private:
		const vulkan_context* context_{};
		vk::RenderPass renderpass_{};
		uint4 render_extent_{};

		renderpass_clear_flags clear_flags_{};
		float4 clear_colour_{};
		float_t depth_{};
		uint32_t stencil_{};

		bool has_previous_pass_{};
		bool has_next_pass_{};

		renderpass_state state_{};
	};
}
