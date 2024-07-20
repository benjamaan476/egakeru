#include "fence.h"

#include "vulkan_types.h"

namespace egkr
{
	fence::shared_ptr fence::create(const vulkan_context* context, bool is_signaled)
	{
		return std::make_shared<fence>(context, is_signaled);
	}

	fence::fence(const vulkan_context* context, bool is_signaled)
		: context_{ context }, is_signaled_{ is_signaled }
	{
		vk::FenceCreateInfo create_info{};
		if (is_signaled_)
		{
			create_info
				.setFlags(vk::FenceCreateFlagBits::eSignaled);
		}
		fence_ = context_->device.logical_device.createFence(create_info, context_->allocator);
	}

	fence::~fence()
	{
		destroy();
	}

	bool fence::wait(uint64_t timeout)
	{
		if (!is_signaled_)
		{
			const auto result = context_->device.logical_device.waitForFences(fence_, true, timeout);

			switch (result)
			{
			case vk::Result::eSuccess:
					is_signaled_ = true;
					return true;
			case vk::Result::eTimeout:
			case vk::Result::eErrorDeviceLost:
			case vk::Result::eErrorOutOfHostMemory:
			case vk::Result::eErrorOutOfDeviceMemory:
			default:
				break;
			}
		}
		else
		{
			return true;
		}
		return false;
	}

	void fence::reset()
	{
		if (is_signaled_)
		{
			context_->device.logical_device.resetFences(fence_);
			is_signaled_ = false;
		}
	}

	void fence::destroy()
	{
		if (fence_)
		{
			context_->device.logical_device.destroyFence(fence_);
			fence_ = VK_NULL_HANDLE;
		}
	}
}
