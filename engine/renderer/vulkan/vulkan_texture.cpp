#include "vulkan_texture.h"
#include "vulkan_types.h"

namespace egkr
{
	namespace image
	{
		vk::Format channel_count_to_format(uint8_t channel_count, vk::Format default_format)
		{
			switch (channel_count)
			{
			case 1:
				return vk::Format::eR8Unorm;
			case 2:
				return vk::Format::eR8G8Unorm;
			case 3:
				return vk::Format::eR8G8B8Unorm;
			case 4:
				return vk::Format::eR8G8B8A8Unorm;
			default:
				return default_format;
			}
		}

		vulkan_texture::shared_ptr vulkan_texture::create(const vulkan_context* context, uint32_t width_, uint32_t height_, const egkr::texture::properties& properties, bool create_view)
		{
			auto img = std::make_shared<vulkan_texture>(context, width_, height_, properties);

			if (create_view)
			{
				//img->create_view(properties);
			}
			return img;
		}

		vulkan_texture* vulkan_texture::create_raw(const vulkan_context* context, uint32_t width, uint32_t height, const egkr::texture::properties& properties, bool create_view)
		{
			auto img = new vulkan_texture(context, width, height, properties);

			if (create_view)
			{
				//img->create_view(properties);
			}
			return img;
		}

		void vulkan_texture::create_view(const properties& properties)
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

		vulkan_texture::vulkan_texture(const vulkan_context* context, uint32_t width_, uint32_t height_, const egkr::texture::properties& properties)
			: texture(properties), context_{ context }, width_{ width_ }, height_{ height_ }
		{

		}

		vulkan_texture::~vulkan_texture()
		{
			free();
		}

		void vulkan_texture::create(const image::properties& properties)
		{
			ZoneScoped;

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

			create_view(properties);

		}

		bool vulkan_texture::populate(const egkr::texture::properties& properties, const uint8_t* data)
		{
			ZoneScoped;

			//if (data)
			{
				vk::DeviceSize image_size = properties.width * properties.height * properties.channel_count;

				auto image_format = vk::Format::eR8G8B8A8Unorm;

				image::properties image_properties{};
				image_properties.tiling = vk::ImageTiling::eOptimal;
				image_properties.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
				image_properties.memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				image_properties.image_format = image_format;
				image_properties.aspect_flags = vk::ImageAspectFlagBits::eColor;

				create(image_properties);

				if (data)
				{
					write_data(0, image_size, data);
				}


				increment_generation();

				return true;
			}
			return false;
		}

		bool vulkan_texture::populate_writeable()
		{
			ZoneScoped;

			{
				image::properties properties{};
				properties.tiling = vk::ImageTiling::eOptimal;
				properties.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
				properties.memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				properties.aspect_flags = vk::ImageAspectFlagBits::eColor;
				properties.image_format = channel_count_to_format(properties_.channel_count, vk::Format::eR8G8B8A8Unorm);

				create(properties);

				increment_generation();

				return true;
			}
			return false;
		}

		void vulkan_texture::free()
		{
			ZoneScoped;

			destroy();
		}

		void vulkan_texture::set_view(vk::ImageView view)
		{
			ZoneScoped;

			const auto& logical_device = context_->device.logical_device;
			logical_device.waitIdle();
			if (view_)
			{
				logical_device.destroyImageView(view_, context_->allocator);
				view_ = VK_NULL_HANDLE;
			}
			view_ = view;
		}

		bool vulkan_texture::write_data(uint64_t offset, uint32_t /*size*/, const uint8_t* data)
		{
			ZoneScoped;

			auto image_size = properties_.width * properties_.height * properties_.channel_count;
			auto image_format = channel_count_to_format(properties_.channel_count, vk::Format::eR8G8B8A8Unorm);

			auto staging_buffer = buffer::create(context_, image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, true);
			staging_buffer->load_data(offset, image_size, 0, data);
			command_buffer single_use{};
			single_use.begin_single_use(context_, context_->device.graphics_command_pool);

			transition_layout(single_use, image_format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
			copy_from_buffer(single_use, staging_buffer);
			transition_layout(single_use, image_format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

			single_use.end_single_use(context_, context_->device.graphics_command_pool, context_->device.graphics_queue);
			return true;
		}

		bool vulkan_texture::resize(uint32_t width, uint32_t height)
		{
			ZoneScoped;

			destroy();

			image::properties properties{};
			properties.tiling = vk::ImageTiling::eOptimal;
			properties.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
			properties.memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			properties.aspect_flags = vk::ImageAspectFlagBits::eColor;
			properties.image_format = channel_count_to_format(properties_.channel_count, vk::Format::eR8G8B8A8Unorm);

			width_ = width;
			height_ = height;

			create(properties);

			create_view(properties);

			increment_generation();
			return true;

		}

		void vulkan_texture::destroy()
		{
			ZoneScoped;

			if (context_)
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
				if ((int)(properties_.flags & egkr::texture::flags::is_wrapped) == 0)
				{

					if (image_)
					{
						logical_device.destroyImage(image_, context_->allocator);
						image_ = VK_NULL_HANDLE;
					}

				}
				context_ = nullptr;
			}
		}

		void vulkan_texture::transition_layout(command_buffer command_buffer, vk::Format /*format*/, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
		{
			ZoneScoped;

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

		void vulkan_texture::copy_from_buffer(command_buffer command_buffer, buffer::shared_ptr buffer)
		{
			ZoneScoped;

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

	namespace texture::vulkan::texture_map
	{
		vk::SamplerAddressMode convert_repeat_type(std::string_view axis, egkr::texture_map::repeat repeat)
		{
			ZoneScoped;
			switch (repeat)
			{
			case egkr::texture_map::repeat::repeat:
				return vk::SamplerAddressMode::eRepeat;
			case egkr::texture_map::repeat::mirrored_repeat:
				return vk::SamplerAddressMode::eMirroredRepeat;
			case egkr::texture_map::repeat::clamp_to_edge:
				return vk::SamplerAddressMode::eClampToEdge;
			case egkr::texture_map::repeat::clamp_to_border:
				return vk::SamplerAddressMode::eClampToBorder;
			default:
				LOG_WARN("Unknown address mode specified for {}. Defaulting to repeat.", axis.data());
				return vk::SamplerAddressMode::eRepeat;
			}
		}

		vk::Filter convert_filter_type(std::string_view op, egkr::texture_map::filter filter)
		{
			ZoneScoped;

			switch (filter)
			{
			case egkr::texture_map::filter::nearest:
				return vk::Filter::eNearest;
			case egkr::texture_map::filter::linear:
				return vk::Filter::eLinear;
			default:
				LOG_WARN("Unknown filter tyoe for {}. Defaulting to nearest", op.data());
				return vk::Filter::eNearest;
			}
		}



		texture_map::texture_map(const vulkan_context* context, const egkr::texture_map::properties& properties)
			: egkr::texture_map::texture_map(properties), context_{ context }
		{}

		void texture_map::acquire()
		{
			vk::SamplerCreateInfo create_info{};
			create_info
				.setMinFilter(convert_filter_type("min", minify))
				.setMagFilter(convert_filter_type("mag", magnify))
				.setAddressModeU(convert_repeat_type("u", repeat_u))
				.setAddressModeV(convert_repeat_type("v", repeat_u))
				.setAddressModeW(convert_repeat_type("w", repeat_u))
				.setAnisotropyEnable(true)
				.setMaxAnisotropy(16)
				.setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
				.setUnnormalizedCoordinates(false)
				.setCompareEnable(false)
				.setMipmapMode(vk::SamplerMipmapMode::eLinear);

			sampler = context_->device.logical_device.createSampler(create_info, context_->allocator);
		}

		void texture_map::release()
		{
			if (context_)
			{
				texture.reset();

				if (sampler)
				{
					context_->device.logical_device.destroySampler(sampler, context_->allocator);
					sampler = VK_NULL_HANDLE;
				}

				context_ = nullptr;
			}
		}

		texture_map::~texture_map()
		{
			release();
		}
	}
}