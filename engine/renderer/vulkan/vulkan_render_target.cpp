#include "vulkan_render_target.h"
#include "vulkan_renderpass.h"
#include "vulkan_types.h"

namespace egkr::render_target
{
	render_target::shared_ptr vulkan_render_target::create(const vulkan_context* context)
	{
		return std::make_shared<vulkan_render_target>(context);
	}

	vulkan_render_target::~vulkan_render_target()
	{
		free(true);
	}

	bool vulkan_render_target::populate(egkr::vector<texture::texture::shared_ptr> attachment, renderpass::renderpass* renderpass, uint32_t width, uint32_t height)
	{
		egkr::vector<vk::ImageView> image_views{};
		attachments_ = attachment;
		for (const auto& attachment : attachments_)
		{
			image_views.push_back(((image::vulkan_texture*)(attachment.get()))->get_view());
		}
		vk::FramebufferCreateInfo create_info{};
		create_info
			.setRenderPass(((renderpass::vulkan_renderpass*)renderpass)->get_handle())
			.setAttachments(image_views)
			.setWidth(width)
			.setHeight(height)
			.setLayers(1);


		framebuffer_ = context_->device.logical_device.createFramebuffer(create_info, context_->allocator);
		return true;
	}
	
	bool vulkan_render_target::free(bool free_internal_memory)
	{
		if (framebuffer_)
		{
			context_->device.logical_device.destroyFramebuffer(framebuffer_);
			framebuffer_ = VK_NULL_HANDLE;
		}

		if (free_internal_memory)
		{
			for (auto& attachment : attachments_)
			{
				attachment->destroy();
			}
			attachments_.clear();
		}
		return true;
	}
}