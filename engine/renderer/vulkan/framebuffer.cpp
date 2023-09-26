#include "framebuffer.h"
#include "vulkan_types.h"

namespace egkr
{
	framebuffer::unique_ptr framebuffer::create(const vulkan_context* context, const framebuffer_properties& properties)
	{
		return std::make_unique<framebuffer>(context, properties);
	}

	framebuffer::framebuffer(const vulkan_context* context, const framebuffer_properties& properties)
		:context_{ context }, attachments_{ properties.attachments }, renderpass_{properties.renderpass}, width_{ properties.width_ }, height_{ properties.height_ }
	{
		vk::FramebufferCreateInfo create_info{};
		create_info
			.setAttachments(attachments_)
			.setWidth(width_)
			.setHeight(height_)
			.setLayers(1)
			.setRenderPass(renderpass_->get_handle());

		framebuffer_ = context_->device.logical_device.createFramebuffer(create_info, context_->allocator);
	}

	framebuffer::~framebuffer()
	{
		destroy();
	}

	void framebuffer::destroy()
	{
		context_->device.logical_device.waitIdle();
		context_->device.logical_device.destroyFramebuffer(framebuffer_, context_->allocator);
		framebuffer_ = VK_NULL_HANDLE;
	}
}