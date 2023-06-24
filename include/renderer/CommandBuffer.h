#pragma once

#include "../EngineCore.h"
#include "RendererState.h"
namespace egkr
{

	class CommandBuffer
	{
	public:
		CommandBuffer();
		CommandBuffer(size_t bufferCount);


		void record(uint32_t bufferIndex, uint32_t imageIndex, std::function<void(vk::CommandBuffer, uint32_t)> recordBuffer);


		vk::CommandBuffer operator[] (uint32_t index)
		{
			return commandBuffers[index];
		}

		const vk::CommandBuffer operator[] (uint32_t index) const
		{
			return commandBuffers[index];
		}

	private:

		std::vector<vk::CommandBuffer> commandBuffers;
	};

	class OneTimeCommandBuffer
	{
		std::vector<vk::CommandBuffer> commandBuffers;
	public:
		OneTimeCommandBuffer(auto&& oneTimeCommand) 
		{
			vk::CommandBufferAllocateInfo allocInfo{};
			allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
			allocInfo.setCommandPool(state.commandPool);
			allocInfo.setCommandBufferCount(1);

			commandBuffers = state.device.allocateCommandBuffers(allocInfo);
			for (const auto& commandBuffer : commandBuffers)
			{
				ENGINE_ASSERT(commandBuffer != vk::CommandBuffer{}, "Failed to allocate command buffer");

			}
			vk::CommandBufferBeginInfo beginInfo{};
			beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

			commandBuffers[0].begin(beginInfo);

			std::invoke(oneTimeCommand, commandBuffers[0]);
			//oneTimeCommand(commandBuffers[0]);

			commandBuffers[0].end();

			vk::SubmitInfo submitInfo{};
			submitInfo.setCommandBufferCount(1);
			submitInfo.setCommandBuffers(commandBuffers[0]);

			state.graphicsQueue.submit(submitInfo, vk::Fence{});
			state.graphicsQueue.waitIdle();
		}


		~OneTimeCommandBuffer() { destroy(); }

		void destroy()
		{
			state.device.freeCommandBuffers(state.commandPool, commandBuffers);
		}
	};
}