#include "swapchain.h"
#include "vulkan_types.h"
#include "systems/texture_system.h"
#include "image.h"

namespace egkr
{
	swapchain::shared_ptr swapchain::create(vulkan_context* context)
	{
		return std::make_shared<swapchain>(context);
	}

	swapchain::swapchain(vulkan_context* context)
		: context_{context}
	{
		create();
	}

	swapchain::~swapchain()
	{
	//	destroy();
	}

	void swapchain::destroy()
	{
		render_targets_.clear();

		if (depth_attachment_)
		{
			depth_attachment_->destroy();
			depth_attachment_.reset();
		}

		for (auto& texture : render_textures_)
		{
			if (texture->data)
			{
				delete (image::image*)texture->data;
			}
		}
		render_textures_.clear();

		image_count_ = 0;

		if (swapchain_)
		{
			context_->device.logical_device.destroySwapchainKHR(swapchain_, context_->allocator);
		}
	}

	void swapchain::recreate()
	{
		destroy();
		create();
	}

	uint32_t swapchain::acquire_next_image_index(vk::Semaphore semaphore, vk::Fence fence)
	{
		uint32_t image_index = 0;
		auto result = context_->device.logical_device.acquireNextImageKHR(swapchain_, UINT64_MAX, semaphore, fence, &image_index);

		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			recreate();
			return -1;
		}
		else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			LOG_ERROR("Failed to acquire next image");
		}

		return image_index;
	}

	void swapchain::present(vk::Queue /*graphics_queue*/, vk::Queue present_queue, vk::Semaphore render_complete, uint32_t image_index)
	{
		vk::PresentInfoKHR present_info{};
		present_info
			.setWaitSemaphores(render_complete)
			.setSwapchains(swapchain_)
			.setImageIndices(image_index);

		auto result = present_queue.presentKHR(present_info);

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
		{
			recreate();
		}
		else if (result != vk::Result::eSuccess)
		{
			LOG_FATAL("Failed to present swap chain image");
		}

		context_->current_frame = (context_->current_frame + 1) % max_frames_in_flight_;
	}

	void swapchain::create()
	{
		auto swapchain_support = query_swapchain_support(context_->surface, context_->device.physical_device);

		format_ = choose_swapchain_surface_format(swapchain_support.formats);
		auto present_mode = choose_swapchain_present_mode(swapchain_support.present_modes);
		extent_ = choose_swapchain_extent(swapchain_support.capabilities);

		image_count_ = swapchain_support.capabilities.minImageCount + 1;
		max_frames_in_flight_ = image_count_ - 1;
		render_targets_.resize(image_count_);

		if (swapchain_support.capabilities.maxImageCount > 0 && image_count_ > swapchain_support.capabilities.maxImageCount)
		{
			image_count_ = swapchain_support.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR create_info{};
		create_info
			.setSurface(context_->surface)
			.setMinImageCount(image_count_)
			.setImageFormat(format_.format)
			.setImageColorSpace(format_.colorSpace)
			.setImageExtent(extent_)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

		auto queue_indices = find_queue_families(*context_, context_->device.physical_device);
		if (!queue_indices.graphics_family.has_value() || !queue_indices.present_family.has_value())
		{
			LOG_ERROR("Failed to find valid queues");
			return;
		}

		std::array<uint32_t, 2> queue_family_indices = { queue_indices.graphics_family.value(), queue_indices.present_family.value() };

		if (queue_indices.graphics_family != queue_indices.present_family)
		{
			create_info.setImageSharingMode(vk::SharingMode::eConcurrent);
			create_info.setQueueFamilyIndexCount(2);
			create_info.pQueueFamilyIndices = queue_family_indices.data();
		}
		else
		{
			create_info.setImageSharingMode(vk::SharingMode::eExclusive);
			create_info.setQueueFamilyIndexCount(0);
			create_info.pQueueFamilyIndices = nullptr;
		}
		create_info.setPreTransform(swapchain_support.capabilities.currentTransform);
		create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		create_info.setPresentMode(present_mode);
		create_info.setClipped((vk::Bool32)true);

		swapchain_ = context_->device.logical_device.createSwapchainKHR(create_info, context_->allocator);

		if (swapchain_ == vk::SwapchainKHR{})
		{
			LOG_ERROR("Failed to create swapchain");
		}

		get_swapchain_images(swapchain_);
	}

	vk::SurfaceFormatKHR swapchain::choose_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats)
	{
		for (const auto& available_format : available_formats)
		{
			if (available_format.format == vk::Format::eB8G8R8A8Unorm && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return available_format;
			}
		}
		LOG_WARN("No ideal format found, picking first format");
		return available_formats[0];
	}

	vk::PresentModeKHR swapchain::choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes)
	{
		for (const auto& available_present_mode : available_present_modes)
		{
			if (available_present_mode == vk::PresentModeKHR::eMailbox)
			{
				return available_present_mode;
			}
		}

		LOG_WARN("No ideal present mode found, picking first mode");
		return available_present_modes[0];
	}

	vk::Extent2D swapchain::choose_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{

			vk::Extent2D actual_extent = {context_->framebuffer_width, context_->framebuffer_height};

			actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actual_extent;
		}
	}

	std::vector<vk::Image> swapchain::get_swapchain_images(vk::SwapchainKHR swapchain)
	{
		std::vector<vk::Image> swapchain_images;
		auto images = context_->device.logical_device.getSwapchainImagesKHR(swapchain);
		image_count_ = images.size();
		for (const auto& image : images)
		{
			swapchain_images.emplace_back(image);
		}

		if (render_textures_.empty())
		{
			render_textures_.resize(image_count_);

			for (auto i{ 0U }; i < image_count_; ++i)
			{
				void* internal_data = malloc(sizeof(image::image));
				std::string name{ "__internal_vulkan_swapchain_image_0__" };
				name[34] = '0' + (char)i;

				render_textures_[i] = texture_system::wrap_internal(name, extent_.width, extent_.height, 4, false, true, false, internal_data);
			}
		}
		else
		{
			for (auto i{ 0U }; i < image_count_; ++i)
			{
				texture_system::resize(render_textures_[i].get(), extent_.width, extent_.height, false);
			}
		}

		for (auto i{ 0U }; i < image_count_; ++i)
		{
			auto img = (image::image*)render_textures_[i]->data;
			img->set_image(swapchain_images[i]);
			img->set_width(extent_.width);
			img->set_height(extent_.height);
		}

		for (auto i{ 0U }; i < image_count_; ++i)
		{
			auto img = (image::image*)render_textures_[i]->data;
			vk::ImageSubresourceRange subresource{};
			subresource
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1);

			vk::ImageViewCreateInfo image_view_info{};
			image_view_info
				.setImage(img->get_image())
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(format_.format)
				.setSubresourceRange(subresource);

			img->set_view(context_->device.logical_device.createImageView(image_view_info, context_->allocator));
		}

		auto has_valid_depth = detect_depth_format(context_->device);
		if (!has_valid_depth)
		{
			LOG_FATAL("No valid depth format found");
		}

		image::properties depth_image_properties{};
		depth_image_properties.image_format = context_->device.depth_format;
		depth_image_properties.tiling = vk::ImageTiling::eOptimal;
		depth_image_properties.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		depth_image_properties.memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		depth_image_properties.aspect_flags = vk::ImageAspectFlagBits::eDepth;

		auto img = image::image::create_raw(context_, extent_.width, extent_.height, depth_image_properties, true);

		depth_attachment_ = texture_system::wrap_internal("__default_depth_texture__", extent_.width, extent_.height, context_->device.depth_channel_count, false, true, false, img);
		return swapchain_images;
	}

}