#include "vulkan_renderpass.h"
#include "vulkan_render_target.h"
#include "command_buffer.h"
#include "vulkan_types.h"

namespace egkr::renderpass
{
	vulkan_renderpass::shared_ptr vulkan_renderpass::create(const renderer_backend* renderer, vulkan_context* context, const egkr::renderpass::configuration& configuration)
	{
		return std::make_shared<vulkan_renderpass>(renderer, context, configuration);
	}

	vulkan_renderpass::vulkan_renderpass(const renderer_backend* renderer, vulkan_context* context, const egkr::renderpass::configuration& configuration)
		: renderpass{renderer, configuration}, context_{ context } 
	{
		render_area_ = configuration.render_area;
		clear_flags_ = configuration.clear_flags;
		clear_colour_ = configuration.clear_colour;
		has_previous_ = !configuration.previous_name.empty();
		has_next_ = !configuration.next_name.empty();
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

	bool vulkan_renderpass::populate(float depth, float stencil, bool has_previous, bool has_next)
	{
		ZoneScoped;

		depth_ = depth;
		stencil_ = stencil;

		const bool do_clear = (clear_flags_ & egkr::renderpass::clear_flags::colour) != 0;
		const bool do_depth = (clear_flags_ & egkr::renderpass::clear_flags::depth) != 0;

		vk::AttachmentDescription colour_attachment{};
		colour_attachment
			.setFormat(context_->swapchain->get_format().format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(do_clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(has_previous? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eUndefined)
			.setFinalLayout(has_next? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::ePresentSrcKHR);

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
				.setLoadOp(has_previous_ ? do_clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare : vk::AttachmentLoadOp::eDontCare)
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(has_previous_ ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eUndefined)
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
		return true;
	}

	bool vulkan_renderpass::begin(render_target::render_target* render_target) const
	{
		ZoneScoped;
		auto vulkan_render_target = (render_target::vulkan_render_target*)render_target;
		auto& command_buffer = context_->graphics_command_buffers[context_->image_index];

		vk::Rect2D render_area{};
		render_area
			.setOffset({ (int32_t)render_area_.x, (int32_t)render_area_.y })
			.setExtent({ (uint32_t)render_area_.z, (uint32_t)render_area_.w });
		vk::RenderPassBeginInfo begin_info{};
		begin_info
			.setRenderPass(renderpass_)
			.setFramebuffer(vulkan_render_target->get_framebuffer())
			.setRenderArea(render_area);
			
		egkr::vector<vk::ClearValue> clear_colours{};
		const bool do_clear_colour = clear_flags_ & egkr::renderpass::clear_flags::colour;

		vk::ClearValue clear_colour{};
		if (do_clear_colour)
		{
			clear_colour.setColor(std::array<float, 4>{clear_colour_.r, clear_colour_.g, clear_colour_.b, clear_colour_.a, });
		}
		clear_colours.push_back(clear_colour);

		const bool do_clear_depth = clear_flags_ & egkr::renderpass::clear_flags::depth;
		const bool do_clear_stencil = clear_flags_ & egkr::renderpass::clear_flags::stencil;
		if (do_clear_depth)
		{
			vk::ClearValue clear_depth{};
			clear_depth.setDepthStencil({ depth_, do_clear_stencil ? stencil_ : 0 });
			clear_colours.push_back(clear_depth);
		}

		begin_info.setClearValues(clear_colours);

		command_buffer.begin_render_pass(begin_info);
		return true;
	}

	bool vulkan_renderpass::end()
	{
		ZoneScoped;

		auto& command_buffer = context_->graphics_command_buffers[context_->image_index];
		command_buffer.end_render_pass();
		return true;
	}

	void vulkan_renderpass::free()
	{
		destroy();
	}
}