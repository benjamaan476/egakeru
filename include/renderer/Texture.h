#pragma once

#include "../EngineCore.h"
#include "RendererState.h"

#include <filesystem>

namespace egkr
{
	struct TextureProperties
	{
		vk::Format format;
		vk::ImageTiling tiling;
		vk::ImageUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
		vk::ImageAspectFlags aspect;
	};

	class Texture
	{
	public:
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		virtual vk::Format getFormat() const = 0;
		virtual vk::DescriptorImageInfo getDescriptor() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		vk::Image image{};
		vk::DeviceMemory memory{};
		vk::ImageView view{};
		uint32_t _width{};
		uint32_t _height{};

		vk::DescriptorImageInfo _imageInfo{};

	public:
		Texture2D() {}
		Texture2D(vk::Image image, vk::Format format, vk::Sampler sampler)
			: image{ image }
		{
			_properties.format = format;
			_properties.aspect = vk::ImageAspectFlagBits::eColor;

			createImageView();

			_imageInfo
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(view)
				.setSampler(sampler);

		}
		Texture2D(uint32_t width, uint32_t height, TextureProperties properties, vk::Sampler sampler)
			: _width{ width }, _height{ height }, _properties{ properties }
		{
			createImage();
			createMemory();
			createImageView();


			_imageInfo
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(view)
				.setSampler(sampler);
		}
		virtual ~Texture2D()
		{

		}

		static Texture2D createFromFile(std::filesystem::path filepath);


		uint32_t getWidth() const override { return _width; }
		uint32_t getHeight() const override { return _height; }
		vk::Format getFormat() const override { return _properties.format; }
		vk::DescriptorImageInfo getDescriptor() const override { return _imageInfo; }

		void transitionLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		void copyFromBuffer(vk::Buffer buffer)
		{
			auto commandBuffer = beginSingleTimeCommand();

			vk::BufferImageCopy region{};
			region.setBufferOffset(0);
			region.setBufferRowLength(0);
			region.setBufferImageHeight(0);

			region.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
			region.imageSubresource.setMipLevel(0);
			region.imageSubresource.setBaseArrayLayer(0);
			region.imageSubresource.setLayerCount(1);

			region.setImageOffset({ 0, 0, 0 });
			region.setImageExtent({ _width, _height, 1 });

			commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
			endSingleTimeCommand(commandBuffer);
		}

		void destroyView()
		{
			state.device.destroyImageView(view);
		}

		void destroy()
		{
			destroyView();
			state.device.freeMemory(memory);
			state.device.destroyImage(image);
		}

	protected:
		TextureProperties _properties{};
		void createImage();
		void createMemory();
		void createImageView();

	private:

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

		bool hasStencilComponent(vk::Format format) const
		{
			return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
		}

		vk::CommandBuffer beginSingleTimeCommand()
		{
			vk::CommandBufferAllocateInfo allocInfo{};
			allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
			allocInfo.setCommandPool(state.commandPool);
			allocInfo.setCommandBufferCount(1);

			auto commandBuffers = state.device.allocateCommandBuffers(allocInfo);
			for (const auto& commandBuffer : commandBuffers)
			{
				ENGINE_ASSERT(commandBuffer != vk::CommandBuffer{}, "Failed to allocate command buffer");

			}
			vk::CommandBufferBeginInfo beginInfo{};
			beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

			commandBuffers[0].begin(beginInfo);
			return commandBuffers[0];
		}

		void endSingleTimeCommand(vk::CommandBuffer commandBuffer)
		{
			commandBuffer.end();

			vk::SubmitInfo submitInfo{};
			submitInfo.setCommandBufferCount(1);
			submitInfo.setCommandBuffers(commandBuffer);

			state.graphicsQueue.submit(submitInfo, vk::Fence{});
			state.graphicsQueue.waitIdle();

			state.device.freeCommandBuffers(state.commandPool, commandBuffer);
		}

	};

	class DepthImage : public Texture2D
	{
	public:
		DepthImage() {}
		DepthImage(RendererState rendererState, uint32_t w, uint32_t h)
		{
			state = rendererState;
			_width = w;
			_height = h;
			_properties =
			{
				.format = state.findDepthFormat(),
				.tiling = vk::ImageTiling::eOptimal,
				.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
				.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
				.aspect = vk::ImageAspectFlagBits::eDepth
			};

			createImage();
			createMemory();
			createImageView();
		}
	};
}