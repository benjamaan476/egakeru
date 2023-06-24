#pragma once

#include "../EngineCore.h"
#include "RendererState.h"

#include <vulkan/vulkan.hpp>
namespace egkr
{
	struct BufferProperties
	{
		vk::DeviceSize size;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags memoryProperties;
	};

	class Buffer
	{
	public:
		Buffer();
		Buffer(BufferProperties properties);
		void map(auto&& bufferData)
		{
			auto data = state.device.mapMemory(memory, 0, properties.size);
			std::memcpy(data, bufferData, properties.size);
			state.device.unmapMemory(memory);
		}
		void copy(Buffer srcBuffer);
		void destroy();

		BufferProperties properties{};

		vk::DeviceMemory memory{};
		vk::Buffer buffer{};

		vk::DescriptorBufferInfo _descriptor{};
	};

}