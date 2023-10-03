#include "vulkan_texture.h"
#include "vulkan_types.h"

namespace egkr
{
	vulkan_texture::shared_ptr vulkan_texture::create(const vulkan_context* context, const texture_properties& properties, const uint8_t* data)
	{
		return std::make_shared<vulkan_texture>(context, properties, data);
	}

	vulkan_texture::vulkan_texture(const vulkan_context* context, const texture_properties& properties, const uint8_t* data)
	: texture(properties), context_{context}
	{
		if (data)
		{
			vk::DeviceSize image_size = properties.width * properties.height * properties.channel_count;
			auto staging_buffer = buffer::create(context_, image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, true);
			staging_buffer->load_data(0, image_size, 0, data);

			image_properties image_properties{};
			image_properties.tiling = vk::ImageTiling::eOptimal;
			image_properties.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
			image_properties.memory_properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			image_properties.image_format = vk::Format::eR8G8B8A8Srgb;
			image_properties.aspect_flags = vk::ImageAspectFlagBits::eColor;

			image_ = image::create(context, properties.width, properties.height, image_properties, true);

			command_buffer single_use{};
			single_use.begin_single_use(context, context->device.graphics_command_pool);

			image_->transition_layout(single_use, vk::Format::eR8G8B8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
			image_->copy_from_buffer(single_use, staging_buffer);
			image_->transition_layout(single_use, vk::Format::eR8G8B8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

			single_use.end_single_use(context, context->device.graphics_command_pool, context->device.graphics_queue);

			vk::SamplerCreateInfo sampler_info{};
			sampler_info
				.setMinFilter(vk::Filter::eLinear)
				.setMagFilter(vk::Filter::eLinear)
				.setAddressModeU(vk::SamplerAddressMode::eRepeat)
				.setAddressModeV(vk::SamplerAddressMode::eRepeat)
				.setAddressModeW(vk::SamplerAddressMode::eRepeat)
				.setAnisotropyEnable(true)
				.setMaxAnisotropy(16)
				.setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
				.setUnnormalizedCoordinates(false)
				.setCompareEnable(false)
				.setMipmapMode(vk::SamplerMipmapMode::eLinear);

			sampler_ = context->device.logical_device.createSampler(sampler_info, context->allocator);
		}
	}

	vulkan_texture::~vulkan_texture()
	{
		destroy();
	}

	void vulkan_texture::destroy()
	{
		context_->device.logical_device.waitIdle();
		if (sampler_)
		{
			context_->device.logical_device.destroySampler(sampler_, context_->allocator);
			sampler_ = VK_NULL_HANDLE;
		}
		if (image_)
		{
			image_.reset();
		}
	}
}