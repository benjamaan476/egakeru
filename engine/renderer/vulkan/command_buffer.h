#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct vulkan_context;
	enum class command_buffer_state
	{
		ready,
		recording,
		in_render_pass,
		recording_ended,
		submitted,
		not_allocated
	};

	class command_buffer
	{
	public:
		void allocate(const vulkan_context* context, vk::CommandPool pool, bool is_primary);
		void free(const vulkan_context* context, vk::CommandPool pool);

		void begin(bool is_single_use, bool is_renderpass_continue, bool is_simultaneous_use) const;
		void end() const;

		void update_submitted();
		void reset();

		void begin_single_use(const vulkan_context* context, vk::CommandPool pool);
		void end_single_use(const vulkan_context* context, vk::CommandPool pool, vk::Queue queue);

		void begin_render_pass(const vk::RenderPassBeginInfo& renderpass_info) const;
		void end_render_pass() const;

		auto& get_handle() const { return command_buffer_; }
	private:
		vk::CommandBuffer command_buffer_{};
		command_buffer_state state_{command_buffer_state::not_allocated};
	};
}
