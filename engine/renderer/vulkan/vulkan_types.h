#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>

#include "swapchain.h"
#include "renderpass.h"
#include "command_buffer.h"
#include "fence.h"
#include "shader.h"
#include "buffer.h"

namespace egkr
{
	struct vulkan_device
	{
		vk::PhysicalDevice physical_device{};
		vk::Device logical_device{};
		vk::Format depth_format{};

		uint32_t graphics_queue_index{};
		vk::Queue graphics_queue{};

		uint32_t present_queue_index{};
		vk::Queue present_queue{};

		vk::CommandPool graphics_command_pool{};

		int32_t find_memory_index(uint32_t type_filter, vk::MemoryPropertyFlags property_flags) const;
	};

	struct vulkan_context
	{
		vk::Instance instance{};
		vk::AllocationCallbacks* allocator = nullptr;
		VkDebugUtilsMessengerEXT debug{};
		vulkan_device device{};
		vk::SurfaceKHR surface{};
		swapchain::shared_ptr swapchain{};
		renderpass::shared_ptr main_renderpass{};

		egkr::vector<command_buffer> graphics_command_buffers{};

		egkr::vector<vk::Semaphore> image_available_semaphore{};
		egkr::vector<vk::Semaphore> queue_complete_semaphore{};

		egkr::vector<fence::shared_ptr> in_flight_fences{};
		egkr::vector<fence::shared_ptr> images_in_flight{};

		uint32_t framebuffer_width{};
		uint32_t framebuffer_height{};
		uint32_t image_index{};
		uint32_t current_frame{};

		bool recreating_swapchain{};
		uint32_t framebuffer_size_generation{};
		uint32_t framebuffer_last_size_generation{};

		shader::shared_ptr object_shader{};

		buffer::shared_ptr object_vertex_buffer{};
		buffer::shared_ptr object_index_buffer{};
		uint64_t geometry_vertex_offset{};
		uint64_t geometry_index_offset{};

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

	struct vertex_3d
	{
		//Must match the vertex input attribute description
		egkr::float3 position{};

		consteval static vk::VertexInputBindingDescription get_binding_description() noexcept
		{
			vk::VertexInputBindingDescription bindingDescription{};
			bindingDescription
				.setBinding(0)
				.setStride(sizeof(vertex_3d))
				.setInputRate(vk::VertexInputRate::eVertex);
			return bindingDescription;
		}

		consteval static std::array<vk::VertexInputAttributeDescription, 1> get_attribute_description() noexcept
		{
			std::array<vk::VertexInputAttributeDescription, 1> attributeDescriptions{};

			attributeDescriptions[0].setBinding(0);
			attributeDescriptions[0].setLocation(0);
			attributeDescriptions[0].setFormat(vk::Format::eR32G32B32A32Sfloat);
			attributeDescriptions[0].setOffset(offsetof(vertex_3d, position));

			//attributeDescriptions[1].setBinding(0);
			//attributeDescriptions[1].setLocation(1);
			//attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
			//attributeDescriptions[1].setOffset(offsetof(vertex_3d, normal));

			//attributeDescriptions[2].setBinding(0);
			//attributeDescriptions[2].setLocation(2);
			//attributeDescriptions[2].setFormat(vk::Format::eR32G32Sfloat);
			//attributeDescriptions[2].setOffset(offsetof(vertex_3d, tex));

			//attributeDescriptions[3].setBinding(0);
			//attributeDescriptions[3].setLocation(3);
			//attributeDescriptions[3].setFormat(vk::Format::eR32G32B32Sfloat);
			//attributeDescriptions[3].setOffset(offsetof(vertex_3d, colour));
			return attributeDescriptions;
		}
	};

}
