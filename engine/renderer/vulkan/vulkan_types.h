#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>

#include "swapchain.h"

namespace egkr
{
	struct vulkan_device
	{
		vk::PhysicalDevice physical_device{};
		vk::Device logical_device{};
		vk::Format depth_format{};

		int32_t find_memory_index(uint32_t type_filter, vk::MemoryPropertyFlags property_flags) const;
	};

	struct vulkan_context
	{
		vk::Instance instance{};
		vk::AllocationCallbacks* allocator = nullptr;
		VkDebugUtilsMessengerEXT debug{};
		vulkan_device device{};
		vk::SurfaceKHR surface{};
		swapchain::unique_ptr swapchain{};
		uint32_t framebuffer_width{};
		uint32_t framebuffer_height{};
		uint32_t image_index{};
		uint32_t frame_count{};

		bool recreating_swapchain{};
	};


	struct queue_family_indices
	{
		std::optional<uint32_t> graphics_family{};
		std::optional<uint32_t> present_family{};

		bool is_complete() const
		{
			return graphics_family.has_value() && present_family.has_value();
		}
	};

	struct swapchain_support_details
	{
		vk::SurfaceCapabilitiesKHR capabilities{};
		std::vector<vk::SurfaceFormatKHR> formats{};
		std::vector<vk::PresentModeKHR> present_modes{};
	};

	static inline swapchain_support_details query_swapchain_support(const vulkan_context& context, const vk::PhysicalDevice& physical_device)
	{
		swapchain_support_details details;
		details.capabilities = physical_device.getSurfaceCapabilitiesKHR(context.surface);
		details.formats = physical_device.getSurfaceFormatsKHR(context.surface);
		details.present_modes = physical_device.getSurfacePresentModesKHR(context.surface);

		return details;
	}

	static inline bool detect_depth_format(vulkan_device& device)
	{
		const std::array<vk::Format, 3> depth_formats{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};

		auto flags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

		for (const auto format : depth_formats)
		{
			const auto properties = device.physical_device.getFormatProperties(format);

			if ((properties.linearTilingFeatures & flags) == flags)
			{
				device.depth_format = format;
				return true;
			}
			else if ((properties.optimalTilingFeatures & flags) == flags)
			{
				device.depth_format = format;
				return true;
			}
		}

		return false;
	}

	static inline queue_family_indices find_queue_families(const vulkan_context& context, const vk::PhysicalDevice& physical_device)
	{
		queue_family_indices queueIndices;
		auto familyProperties = physical_device.getQueueFamilyProperties();
		int index = 0;
		for (const auto& queueFamily : familyProperties)
		{
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				queueIndices.graphics_family = index;
			}
			auto presentSupport = physical_device.getSurfaceSupportKHR(index, context.surface);

			if (presentSupport > 0)
			{
				queueIndices.present_family = index;
			}

			if (queueIndices.is_complete())
			{
				break;
			}

			index++;
		}
		return queueIndices;
	}

}
