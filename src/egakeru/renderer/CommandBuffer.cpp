#include "renderer/CommandBuffer.h"

namespace egkr
{
	CommandBuffer::CommandBuffer() {}

	CommandBuffer::CommandBuffer(size_t bufferCount)
	{
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.setCommandPool(state.commandPool);
		allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocInfo.setCommandBufferCount(static_cast<uint32_t>(bufferCount));

		commandBuffers = state.device.allocateCommandBuffers(allocInfo);

		for (const auto& commandBuffer : commandBuffers)
		{
			ENGINE_ASSERT(commandBuffer != vk::CommandBuffer{}, "Failed to create command buffer");
		}
	}

	void CommandBuffer::record(uint32_t bufferIndex, uint32_t imageIndex, std::function<void(vk::CommandBuffer, uint32_t)> recordBuffer)
	{
		auto& commandBuffer = commandBuffers[bufferIndex];

		commandBuffer.reset();

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.setPInheritanceInfo(nullptr);
		commandBuffer.begin(beginInfo);

		recordBuffer(commandBuffer, imageIndex);

		commandBuffer.end();
	}
}