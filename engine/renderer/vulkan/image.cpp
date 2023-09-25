#include "image.h"
#include "vulkan_types.h"

namespace egkr
{
	vulkan_image::shared_ptr vulkan_image::create(const vulkan_context* context, uint32_t width_, uint32_t height_, const image_properties& properties, bool create_view)
	{
		auto image =  std::make_shared<vulkan_image>(context, width_, height_, properties);

		if (create_view)
		{
			image->create_view(properties);
		}
		return image;
	}

	void vulkan_image::create_view(const image_properties& properties)
	{
		vk::ImageSubresourceRange subresource{};
		subresource
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1)
			.setAspectMask(properties.aspect_flags);


		vk::ImageViewCreateInfo image_view_info{};
		image_view_info
			.setImage(image_)
			.setViewType(vk::ImageViewType::e2D) // TODO configurable
			.setFormat(properties.image_format)
			.setSubresourceRange(subresource);

		view_ = context_->device.logical_device.createImageView(image_view_info, context_->allocator);

	}

	vulkan_image::vulkan_image(const vulkan_context* context, uint32_t width_, uint32_t height_, const image_properties& properties)
		: context_{ context }, width_{width_}, height_{height_}
	{
		vk::ImageCreateInfo image_info{};
	
		image_info
			.setImageType(vk::ImageType::e2D)
			.setExtent({ width_, height_, properties.depth })
			.setFormat(properties.image_format)
			.setMipLevels(properties.mip_levels)
			.setTiling(properties.tiling)
			.setArrayLayers(properties.array_layers)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(properties.usage)
			.setSamples(properties.samples)
			.setSharingMode(properties.sharing_mode);

		image_ = context_->device.logical_device.createImage(image_info, context_->allocator);
		if (image_ == vk::Image{})
		{
			LOG_ERROR("Failed to create vulkan_image");
		}

		const auto memory_requirements = context_->device.logical_device.getImageMemoryRequirements(image_);
		auto memory_type = context_->device.find_memory_index(memory_requirements.memoryTypeBits, properties.memory_properties);
		if (memory_type == -1)
		{
			LOG_ERROR("Required memory type not found");
		}

		vk::MemoryAllocateInfo memory_info{};
		memory_info
			.setAllocationSize(memory_requirements.size)
			.setMemoryTypeIndex(memory_type);

		memory_ = context_->device.logical_device.allocateMemory(memory_info, context_->allocator);

		//TODO conigurable memory offset
		context_->device.logical_device.bindImageMemory(image_, memory_, 0);
	}

	vulkan_image::~vulkan_image()
	{
		destroy();
	}

	void vulkan_image::destroy()
	{
		const auto& logical_device = context_->device.logical_device;
		if (view_)
		{
			logical_device.destroyImageView(view_, context_->allocator);
			view_ = VK_NULL_HANDLE;
		}

		if (memory_)
		{
			logical_device.freeMemory(memory_, context_->allocator);
			memory_ = VK_NULL_HANDLE;
		}

		if (image_)
		{
			logical_device.destroyImage(image_, context_->allocator);
			image_ = VK_NULL_HANDLE;
		}
	}
}