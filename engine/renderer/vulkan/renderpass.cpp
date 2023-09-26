#include "renderpass.h"
#include "command_buffer.h"
#include "vulkan_types.h"

namespace egkr
{
	renderpass::shared_ptr renderpass::create(const vulkan_context* context, const renderpass_properties& properties)
	{
		return std::make_shared<renderpass>(context, properties);
	}

	renderpass::renderpass(const vulkan_context* context, const renderpass_properties& properties)
		: context_{ context }, render_extent_{ properties.render_extent }, clear_colour_{ properties.clear_colour }, depth_{ properties.depth }, stencil_{ properties.stencil }
	{

		vk::AttachmentDescription colour_attachment{};
		colour_attachment
			.setFormat(context_->swapchain->get_format().format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		//TODO not always need a depth attachment
		vk::AttachmentDescription depth_attachment{};
		depth_attachment
			.setFormat(context_->device.depth_format) // TODO configure
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);


		vk::AttachmentReference colour_attachment_reference{};
		colour_attachment_reference
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::AttachmentReference depth_attachment_reference{};
		depth_attachment_reference
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		//TODO other attachment types (input, resolve, preserve)
		//TODO configurable
		std::array<vk::AttachmentDescription, 2> attachment_descriptions{colour_attachment, depth_attachment};

		vk::SubpassDescription subpass{};
		subpass
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(colour_attachment_reference)
			.setPDepthStencilAttachment(&depth_attachment_reference);

		vk::SubpassDependency subpass_dependencies{};
		subpass_dependencies
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo renderpass_info{};
		renderpass_info
			.setAttachments(attachment_descriptions)
			.setSubpasses(subpass)
			.setDependencies(subpass_dependencies);

		renderpass_ = context_->device.logical_device.createRenderPass(renderpass_info, context_->allocator);

	}

	renderpass::~renderpass()
	{
		destroy();
	}

	void renderpass::destroy()
	{
		if (renderpass_)
		{
			context_->device.logical_device.destroyRenderPass(renderpass_, context_->allocator);
			renderpass_ = VK_NULL_HANDLE;
		}

		context_ = nullptr;
	}

	void renderpass::begin(command_buffer& command_buffer, vk::Framebuffer framebuffer)
	{
		vk::Rect2D render_area{};
		render_area
			.setOffset({ (int32_t)render_extent_.x, (int32_t)render_extent_.y })
			.setExtent({ render_extent_.z, render_extent_.w });
	
		vk::ClearValue colour_clear_value{};
		colour_clear_value
			.setColor({ clear_colour_.r, clear_colour_.g, clear_colour_.b, clear_colour_.a });

		vk::ClearValue depth_clear_value{};
		depth_clear_value
			.setDepthStencil({ depth_, stencil_ });

		//Same order as attachments
		std::array<vk::ClearValue, 2> clear_values{colour_clear_value, depth_clear_value};

		vk::RenderPassBeginInfo begin_info{};
		begin_info
			.setRenderPass(renderpass_)
			.setFramebuffer(framebuffer)
			.setClearValues(clear_values)
			.setRenderArea(render_area);

		command_buffer.begin_render_pass(begin_info);
	}

	void renderpass::end(command_buffer& command_buffer)
	{
		command_buffer.end_render_pass();
	}
	void renderpass::set_extent(uint4 extent)
	{
		render_extent_ = extent;
	}
}