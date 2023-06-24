#include "renderer/Texture.h"

#include "Application.h"

#include "renderer/Buffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace egkr
{

	uint32_t Texture2D::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
	{
		auto memoryProperties = state.physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		ENGINE_ASSERT(false, "Failed to find suitable memory type");
		return (uint32_t)-1;
	}

	void Texture2D::createImage()
	{
		vk::ImageCreateInfo imageInfo{};
		imageInfo.setImageType(vk::ImageType::e2D);
		imageInfo.extent.setWidth(_width);
		imageInfo.extent.setHeight(_height);
		imageInfo.extent.setDepth(1);
		imageInfo.setMipLevels(1);
		imageInfo.setArrayLayers(1);
		imageInfo.setFormat(_properties.format);
		imageInfo.setTiling(_properties.tiling);
		imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
		imageInfo.setUsage(_properties.usage);
		imageInfo.setSharingMode(vk::SharingMode::eExclusive);
		imageInfo.setSamples(vk::SampleCountFlagBits::e1);

		image = state.device.createImage(imageInfo);
		ENGINE_ASSERT(image != vk::Image{}, "Failed to create image");
	}
	void Texture2D::createMemory()
	{
		auto memoryRequirements = state.device.getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.setAllocationSize(memoryRequirements.size);
		allocInfo.setMemoryTypeIndex(findMemoryType(memoryRequirements.memoryTypeBits, _properties.memoryProperties));

		memory = state.device.allocateMemory(allocInfo);
		ENGINE_ASSERT(memory != vk::DeviceMemory{}, "Failed to allocate image memory");

		state.device.bindImageMemory(image, memory, 0);

	}
	void Texture2D::createImageView()
	{
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.setImage(image);
		viewInfo.setViewType(vk::ImageViewType::e2D);
		viewInfo.setFormat(_properties.format);
		viewInfo.subresourceRange.setAspectMask(_properties.aspect);
		viewInfo.subresourceRange.setBaseMipLevel(0);
		viewInfo.subresourceRange.setLevelCount(1);
		viewInfo.subresourceRange.setBaseArrayLayer(0);
		viewInfo.subresourceRange.setLayerCount(1);
		view = state.device.createImageView(viewInfo);

		ENGINE_ASSERT(view != vk::ImageView{}, "Failed to create image view");
	}

	Texture2D Texture2D::createFromFile(std::filesystem::path filepath)
	{
		int width{}, height{}, channels{};
		stbi_uc* pixels = nullptr;

		auto searchPaths = getTextureDirectories();

		for (const auto& path : searchPaths)
		{
			auto pathToImage = path / filepath;
			if (std::filesystem::exists(pathToImage))
			{
				pixels = stbi_load(pathToImage.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
				break;
			}
		}

		ENGINE_ASSERT(pixels, "Failed to load image");

		vk::DeviceSize imageSize = width * height * 4;

		BufferProperties properties =
		{
			.size = imageSize,
			.usage = vk::BufferUsageFlagBits::eTransferSrc,
			.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostVisible,
		};

		auto stagingBuffer = Buffer(properties);
		auto data = state.device.mapMemory(stagingBuffer.memory, 0, imageSize);
		std::memcpy(data, pixels, imageSize);
		state.device.unmapMemory(stagingBuffer.memory);

		stbi_image_free(pixels);

		TextureProperties imageProperties
		{
			.format = vk::Format::eB8G8R8A8Srgb,
			.tiling = vk::ImageTiling::eOptimal,
			.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
			.aspect = vk::ImageAspectFlagBits::eColor
		};

		auto textureImage = Texture2D(width, height, imageProperties, egakeru::getSampler());
		textureImage.transitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		textureImage.copyFromBuffer(stagingBuffer.buffer);
		textureImage.transitionLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		state.device.destroyBuffer(stagingBuffer.buffer);
		state.device.freeMemory(stagingBuffer.memory);

		return textureImage;
	}

	void Texture2D::transitionLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		auto commandBuffer = beginSingleTimeCommand();

		vk::ImageMemoryBarrier barrier{};
		barrier.setOldLayout(oldLayout);
		barrier.setNewLayout(newLayout);
		barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setImage(image);
		barrier.subresourceRange.setBaseMipLevel(0);
		barrier.subresourceRange.setLevelCount(1);
		barrier.subresourceRange.setBaseArrayLayer(0);
		barrier.subresourceRange.setLayerCount(1);

		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);

			if (hasStencilComponent(_properties.format))
			{
				barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
			}
		}
		else
		{
			barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
		}
		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			//barrier.setSrcAccessMask(0);
			barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			//barrier.setSrcAccessMask();
			barrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else
		{
			ENGINE_ASSERT(false, "Cannot support this image transition");
		}

		commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags{}, nullptr, nullptr, barrier);

		endSingleTimeCommand(commandBuffer);

	}
}