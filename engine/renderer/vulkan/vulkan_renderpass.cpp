#include "vulkan_renderpass.h"
#include "command_buffer.h"
#include "vulkan_types.h"

namespace egkr
{
	vulkan_renderpass::shared_ptr vulkan_renderpass::create(const vulkan_context* context, const renderpass_properties& properties)
	{
		return std::make_shared<vulkan_renderpass>(context, properties);
	}

	vulkan_renderpass::vulkan_renderpass(const vulkan_context* context, const renderpass_properties& properties)
		: context_{ context }, 
		render_extent_{ properties.render_extent },
		clear_flags_{properties.clear_flags},
		clear_colour_{ properties.clear_colour },
		depth_{ properties.depth },
		stencil_{ properties.stencil },
		has_previous_pass_{ properties.has_previous_pass },
		has_next_pass_{ properties.has_next_pass }
	{

		const bool do_clear = (clear_flags_ & renderpass_clear_flags::colour) != 0;
		const bool do_depth = (clear_flags_ & renderpass_clear_flags::depth) != 0;

		vk::AttachmentDescription colour_attachment{};
		colour_attachment
			.setFormat(context_->swapchain->get_format().format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(do_clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(has_previous_pass_ ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eUndefined)
			.setFinalLayout(has_next_pass_ ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference colour_attachment_reference{};
		colour_attachment_reference
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		//TODO other attachment types (input, resolve, preserve)
		//TODO configurable
		egkr::vector<vk::AttachmentDescription> attachment_descriptions{ colour_attachment };

		if (do_depth)
		{
			vk::AttachmentDescription depth_attachment{};
			depth_attachment
				.setFormat(context_->device.depth_format) // TODO configure
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(has_previous_pass_ ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eUndefined)
				.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);


			attachment_descriptions.push_back(depth_attachment);
		}

		vk::AttachmentReference depth_attachment_reference{};
		depth_attachment_reference
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::SubpassDescription subpass{};
		subpass
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(colour_attachment_reference)
			.setPDepthStencilAttachment(do_depth ? &depth_attachment_reference : nullptr);

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

	vulkan_renderpass::~vulkan_renderpass()
	{
		destroy();
	}

	void vulkan_renderpass::destroy()
	{
		if (renderpass_)
		{
			context_->device.logical_device.destroyRenderPass(renderpass_, context_->allocator);
			renderpass_ = VK_NULL_HANDLE;
		}

		context_ = nullptr;
	}

	void vulkan_renderpass::begin(command_buffer& command_buffer, vk::Framebuffer framebuffer)
	{
		vk::Rect2D render_area{};
		render_area
			.setOffset({ (int32_t)render_extent_.x, (int32_t)render_extent_.y })
			.setExtent({ render_extent_.z, render_extent_.w });
	

		egkr::vector<vk::ClearValue> clear_values{};

		if (clear_flags_ & renderpass_clear_flags::colour)
		{
			vk::ClearValue colour_clear_value{};
			colour_clear_value
				.setColor({ std::array<float, 4>{ clear_colour_.r, clear_colour_.g, clear_colour_.b, clear_colour_.a } });
			clear_values.push_back(colour_clear_value);
		}

		if (clear_flags_ & renderpass_clear_flags::depth)
		{
			vk::ClearValue depth_clear_value{};
			depth_clear_value
				.setDepthStencil({ depth_, stencil_ });
			clear_values.push_back(depth_clear_value);
		}

		vk::RenderPassBeginInfo begin_info{};
		begin_info
			.setRenderPass(renderpass_)
			.setFramebuffer(framebuffer)
			.setClearValues(clear_values)
			.setRenderArea(render_area);

		command_buffer.begin_render_pass(begin_info);
	}

	void vulkan_renderpass::end(command_buffer& command_buffer)
	{
		command_buffer.end_render_pass();
	}
	void vulkan_renderpass::set_extent(uint4 extent)
	{
		render_extent_ = extent;
	}
}