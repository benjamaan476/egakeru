#include "image.h"
#include "vulkan_types.h"

namespace egkr
{
	image::shared_ptr image::create(const vulkan_context* context, uint32_t width_, uint32_t height_, const image_properties& properties, bool create_view)
	{
		auto img =  std::make_shared<image>(context, width_, height_, properties);

		if (create_view)
		{
			img->create_view(properties);
		}
		return img;
	}

	image* image::create_raw(const vulkan_context* context, uint32_t width, uint32_t height, const image_properties& properties, bool create_view)
	{
		auto img = new image(context, width, height, properties);


		if (create_view)
		{
			img->create_view(properties);
		}
		return img;
	}

	void image::create_view(const image_properties& properties)
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

	image::image(const vulkan_context* context, uint32_t width_, uint32_t height_, const image_properties& properties)
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

	image::~image()
	{
		//destroy();
	}

	void image::destroy()
	{
		const auto& logical_device = context_->device.logical_device;
		logical_device.waitIdle();
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

	void image::transition_layout(command_buffer command_buffer, vk::Format /*format*/, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
	{
		vk::ImageSubresourceRange subresource_range{};
		subresource_range
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1)
			.setAspectMask(vk::ImageAspectFlagBits::eColor);

		vk::ImageMemoryBarrier barrier{};
		barrier
			.setOldLayout(old_layout)
			.setNewLayout(new_layout)
			.setSrcQueueFamilyIndex(context_->device.graphics_queue_index)
			.setDstQueueFamilyIndex(context_->device.graphics_queue_index)
			.setImage(image_)
			.setSubresourceRange(subresource_range);

		vk::PipelineStageFlags source_stage{};
		vk::PipelineStageFlags destination_stage{};

		if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
			source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
			destination_stage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier
				.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

			source_stage = vk::PipelineStageFlagBits::eTransfer;
			destination_stage = vk::PipelineStageFlagBits::eFragmentShader;

		}
		else
		{
			LOG_FATAL("Unsupported transition");
		}

		command_buffer.get_handle().pipelineBarrier(source_stage, destination_stage, vk::DependencyFlags{}, nullptr, nullptr, barrier);
	}

	void image::copy_from_buffer(command_buffer command_buffer, buffer::shared_ptr buffer)
	{
		vk::ImageSubresourceLayers subresource{};
		subresource
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(0)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		vk::BufferImageCopy image_copy{};
		image_copy
			.setBufferOffset(0)
			.setBufferRowLength(0)
			.setBufferImageHeight(0)
			.setImageSubresource(subresource)
			.setImageExtent({ width_, height_, 1 });

		command_buffer.get_handle().copyBufferToImage(buffer->get_handle(), image_, vk::ImageLayout::eTransferDstOptimal, image_copy);
	}
}