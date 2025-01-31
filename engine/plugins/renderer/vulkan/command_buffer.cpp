#include "command_buffer.h"
#include "vulkan_types.h"

namespace egkr
{
	void command_buffer::begin_render_pass(const vk::RenderPassBeginInfo& renderpass_info) const
	{
		ZoneScoped;

		command_buffer_.beginRenderPass(renderpass_info, vk::SubpassContents::eInline);
	}
	void command_buffer::end_render_pass() const
	{
		ZoneScoped;

		command_buffer_.endRenderPass();
	}
	void command_buffer::allocate(const vulkan_context* context, vk::CommandPool pool, bool is_primary)
	{
		ZoneScoped;

		vk::CommandBufferAllocateInfo allocate_info{};
		allocate_info
			.setCommandPool(pool)
			.setLevel(is_primary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary)
			.setCommandBufferCount(1);

		state_ = command_buffer_state::not_allocated;

		auto buffers = context->device.logical_device.allocateCommandBuffers(allocate_info);
		command_buffer_ = buffers[0];

		state_ = command_buffer_state::ready;
	}

	void command_buffer::free(const vulkan_context* context, vk::CommandPool pool)
	{
		ZoneScoped;

		if(command_buffer_)
		{
			context->device.logical_device.freeCommandBuffers(pool, command_buffer_);
			command_buffer_ = VK_NULL_HANDLE;
		}
		state_ = command_buffer_state::not_allocated;
	}

	void command_buffer::begin(bool is_single_use, bool is_renderpass_continue, bool is_simultaneous_use) const
	{
		ZoneScoped;

		vk::CommandBufferUsageFlags usage{};

		if (is_single_use)
		{
			usage |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		}

		if (is_renderpass_continue)
		{
			usage |= vk::CommandBufferUsageFlagBits::eRenderPassContinue;
		}

		if (is_simultaneous_use)
		{
			usage |= vk::CommandBufferUsageFlagBits::eSimultaneousUse;
		}

		vk::CommandBufferBeginInfo begin_info{};
		begin_info
			.setFlags(usage);

		command_buffer_.begin(begin_info);
	}

	void command_buffer::end() const
	{
		ZoneScoped;

		command_buffer_.end();
	}

	void command_buffer::update_submitted()
	{
		state_ = command_buffer_state::submitted;
	}

	void command_buffer::reset()
	{
		state_ = command_buffer_state::ready;
	}

	void command_buffer::begin_single_use(const vulkan_context* context, vk::CommandPool pool)
	{
		ZoneScoped;

		allocate(context, pool, true);
		begin(true, false, false);
	}

	void command_buffer::end_single_use(const vulkan_context* context, vk::CommandPool pool, vk::Queue queue)
	{
		ZoneScoped;

		end();

		vk::SubmitInfo submit_info{};
		submit_info
			.setCommandBuffers(command_buffer_);

		queue.submit(submit_info);
		queue.waitIdle();

		free(context, pool);
	}
}
