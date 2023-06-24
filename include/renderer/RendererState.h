#pragma once

#include "../EngineCore.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct RendererState
	{
		vk::Instance instance{};
		vk::AllocationCallbacks allocator{};
		vk::PhysicalDevice physicalDevice{};
		vk::Device device{};
		vk::CommandPool commandPool{};
		vk::Queue graphicsQueue{};
		vk::Queue presentQueue{};
		uint32_t queueFamily{};

		vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
		{
			for (const auto& format : candidates)
			{
				auto properties = physicalDevice.getFormatProperties(format);
				if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
				{
					return format;
				}
				else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)
				{
					return format;
				}
			}
			ENGINE_ASSERT(false, "Failed to find supported format");
			return candidates.front();
		}

		vk::Format findDepthFormat()
		{
			return findSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
		}

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
		{
			auto memoryProperties = physicalDevice.getMemoryProperties();

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
	};

	extern inline RendererState state{};

}