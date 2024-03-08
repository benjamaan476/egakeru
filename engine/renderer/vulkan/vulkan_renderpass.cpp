#include "vulkan_renderpass.h"
#include "vulkan_render_target.h"
#include "command_buffer.h"
#include "vulkan_types.h"

namespace egkr::renderpass
{
	vulkan_renderpass::shared_ptr vulkan_renderpass::create(const vulkan_context* context, const egkr::renderpass::configuration& configuration)
	{
		return std::make_shared<vulkan_renderpass>(context, configuration);
	}

	vulkan_renderpass::vulkan_renderpass(const vulkan_context* context, const egkr::renderpass::configuration& configuration)
		: renderpass{configuration}, context_{ context } 
	{
		vk::SubpassDescription subpasses{};
		subpasses
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

		egkr::vector<vk::AttachmentDescription> colour_attachments{};
		egkr::vector<vk::AttachmentDescription> depth_attachments{};

			for (const auto& attachment_config : configuration.target.attachments)
			{
				vk::AttachmentDescription attach{};
				if (attachment_config.type == render_target::attachment_type::colour)
				{
					bool do_clear_colour = clear_flags_ & clear_flags::colour;
					if (attachment_config.source == render_target::attachment_source::default_source)
					{
						attach.setFormat(context_->swapchain->get_format().format);
					}
					else
					{
						attach.setFormat(vk::Format::eR8G8B8A8Unorm);
					}

					attach.setSamples(vk::SampleCountFlagBits::e1);

					if (attachment_config.load_operation == render_target::load_operation::dont_care)
					{
						attach.setLoadOp(do_clear_colour ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare);
					}
					else
					{
						if (attachment_config.load_operation == render_target::load_operation::load)
						{
							if (do_clear_colour)
							{
								attach.setLoadOp(vk::AttachmentLoadOp::eClear);
							}
							else
							{
								attach.setLoadOp(vk::AttachmentLoadOp::eLoad);
							}
						}
						else
						{
							LOG_FATAL("Invalid combination of load and clear specified");
							return;
						}
					}

					if (attachment_config.store_operation == render_target::store_operation::dont_care)
					{
						attach.setStoreOp(vk::AttachmentStoreOp::eDontCare);
					}
					else if (attachment_config.store_operation == render_target::store_operation::store)
					{
						attach.setStoreOp(vk::AttachmentStoreOp::eStore);
					}
					else
					{
						LOG_FATAL("Invalid store operation");
						return;
					}

					attach.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
					attach.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
					attach.setInitialLayout(attachment_config.load_operation == render_target::load_operation::load ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eUndefined);
					attach.setFinalLayout(attachment_config.present_after ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eColorAttachmentOptimal);

					colour_attachments.push_back(attach);
				}
				else if (attachment_config.type == render_target::attachment_type::depth)
				{
					bool do_clear_depth = clear_flags_ & clear_flags::depth;

					if (attachment_config.source == render_target::attachment_source::default_source)
					{
						attach.setFormat(context_->device.depth_format);
					}
					else
					{
						attach.setFormat(context_->device.depth_format);
					}

					attach.setSamples(vk::SampleCountFlagBits::e1);

					if (attachment_config.load_operation == render_target::load_operation::dont_care)
					{
						attach.setLoadOp(do_clear_depth ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare);
					}
					else
					{
						if (attachment_config.load_operation == render_target::load_operation::load)
						{
							if (do_clear_depth)
							{
								attach.setLoadOp(vk::AttachmentLoadOp::eClear);
							}
							else
							{
								attach.setLoadOp(vk::AttachmentLoadOp::eLoad);
							}
						}
						else
						{
							LOG_FATAL("Invalid combination of load and clear specified");
							return;
						}
					}

					if (attachment_config.store_operation == render_target::store_operation::dont_care)
					{
						attach.setStoreOp(vk::AttachmentStoreOp::eDontCare);
					}
					else if (attachment_config.store_operation == render_target::store_operation::store)
					{
						attach.setStoreOp(vk::AttachmentStoreOp::eStore);
					}
					else
					{
						LOG_FATAL("Invalid store operation");
						return;
					}

					attach.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
					attach.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

					attach.setInitialLayout(attachment_config.load_operation == render_target::load_operation::load ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eUndefined);
					attach.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
					depth_attachments.push_back(attach);
				}
			}

		uint32_t attachment_added{};

		egkr::vector<vk::AttachmentReference> colour_attachment_references{};
		for (auto i{ 0U }; i < colour_attachments.size(); ++i)
		{
			vk::AttachmentReference ref{};
			ref
				.setAttachment(attachment_added)
				.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
			colour_attachment_references.push_back(ref);

			++attachment_added;
		}

		subpasses.setColorAttachments(colour_attachment_references);

		egkr::vector<vk::AttachmentReference> depth_attachment_references{};
		for (auto i{ 0U }; i < depth_attachments.size(); ++i)
		{
			vk::AttachmentReference ref{};
			ref
				.setAttachment(attachment_added)
				.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
			depth_attachment_references.push_back(ref);

			++attachment_added;
		}
		subpasses.setPDepthStencilAttachment(depth_attachment_references.empty() ? nullptr : depth_attachment_references.data());

		vk::SubpassDependency dependencies{};
		dependencies
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

		egkr::vector<vk::AttachmentDescription> attachments{ colour_attachments };
		for (const auto& depth : depth_attachments)
		{
			attachments.push_back(depth);
		}

		vk::RenderPassCreateInfo create_info{};
		create_info
			.setAttachments(attachments)
			.setSubpasses(subpasses)
			.setDependencies(dependencies);

		renderpass_ = context_->device.logical_device.createRenderPass(create_info, context_->allocator);
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

		if (!render_targets_.empty())
		{
			for (auto& target : render_targets_)
			{
				target->destroy();
			}
			render_targets_.clear();
		}

		context_ = nullptr;
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
		else
		{
			for (const auto& target : render_target->get_attachments())
			{
				if (target.type == render_target::attachment_type::depth)
				{
					clear_colours.emplace_back();
				}
			}
		}

		begin_info.setClearValues(clear_colours);

		command_buffer.begin_render_pass(begin_info);
		return true;
	}

	bool vulkan_renderpass::end() const
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