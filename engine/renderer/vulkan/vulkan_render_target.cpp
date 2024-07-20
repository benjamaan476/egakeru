#include "vulkan_render_target.h"
#include "vulkan_renderpass.h"
#include "vulkan_types.h"

namespace egkr::render_target
{
	render_target::shared_ptr vulkan_render_target::create(const vulkan_context* context, const egkr::vector<egkr::render_target::attachment>& attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height)
	{
		return std::make_shared<vulkan_render_target>(context, attachments, renderpass, width, height);
	}

	render_target::shared_ptr vulkan_render_target::create(const vulkan_context* context, const egkr::vector<attachment_configuration>& attachments)
	{
		return std::make_shared<vulkan_render_target>(context, attachments);
	}

	vulkan_render_target::vulkan_render_target(const vulkan_context* context, const egkr::vector<attachment_configuration>& attachments)
		: render_target(), context_{ context }
	{
		for (const auto& configuration : attachments)
		{
			attachment attach
			{
				.type = configuration.type,
				.source = configuration.source,
				.load_operation = configuration.load_operation,
				.store_operation = configuration.store_operation,
				.present_after = configuration.present_after
			};
			attachments_.push_back(attach);
		}
	}

	vulkan_render_target::vulkan_render_target(const vulkan_context* context, const egkr::vector<egkr::render_target::attachment>& attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height)
		: render_target(), context_{context}
	{
		egkr::vector<vk::ImageView> attachment_views{};
		attachments_ = attachments;
		for (const auto& attachment : attachments_)
		{
			attachment_views.push_back(((vulkan_texture*)(attachment.texture))->get_view());
		}
		vk::FramebufferCreateInfo create_info{};
		create_info
			.setRenderPass(((renderpass::vulkan_renderpass*)renderpass)->get_handle())
			.setAttachments(attachment_views)
			.setWidth(width)
			.setHeight(height)
			.setLayers(1);


		framebuffer_ = context_->device.logical_device.createFramebuffer(create_info, context_->allocator);
	}

	vulkan_render_target::~vulkan_render_target()
	{
		free(true);
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
			attachments_.clear();
		}
		return true;
	}
}
