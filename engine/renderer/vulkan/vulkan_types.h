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

	struct physical_device_requirements
	{
		bool graphics{};
		bool present{};
		bool compute{};
		bool transfer{};

		egkr::vector<const char*> extension_names{};
		bool sampler_anisotropy;
		bool discrete_gpu;
	};

	struct physical_device_queue_family_info
	{
		int32_t graphics_index{-1};
		int32_t present_index{-1};
		int32_t compute_index{-1};
		int32_t transfer_index{-1};
	};

	struct swapchain_support_details
	{
		vk::SurfaceCapabilitiesKHR capabilities{};
		std::vector<vk::SurfaceFormatKHR> formats{};
		std::vector<vk::PresentModeKHR> present_modes{};
	};

	struct vulkan_context;
	struct vulkan_device
	{
		vk::PhysicalDevice physical_device{};
		vk::Device logical_device{};
		vk::Format depth_format{};

		uint32_t graphics_queue_index{};
		vk::Queue graphics_queue{};

		uint32_t present_queue_index{};
		vk::Queue present_queue{};

		uint32_t transfer_queue_index{};
		vk::Queue transfer_queue{};

		vk::PhysicalDeviceProperties properties{};
		vk::PhysicalDeviceFeatures features{};
		vk::PhysicalDeviceMemoryProperties memory{};

		vk::CommandPool graphics_command_pool{};

		swapchain_support_details swapchain_supprt{};

		int32_t find_memory_index(uint32_t type_filter, vk::MemoryPropertyFlags property_flags) const;

		bool physical_device_meets_requirements(vk::PhysicalDevice device, vk::SurfaceKHR surface, const vk::PhysicalDeviceProperties& properties, const vk::PhysicalDeviceFeatures features, const physical_device_requirements& requirements, physical_device_queue_family_info& family_info, swapchain_support_details& swapchain_support);
		bool select_physical_device(vulkan_context* context);
		bool create(vulkan_context* context);
	};

	struct vulkan_context
	{
		vk::Instance instance{};
		vk::AllocationCallbacks* allocator = nullptr;
		VkDebugUtilsMessengerEXT debug{};
		vulkan_device device{};
		vk::SurfaceKHR surface{};
		swapchain::shared_ptr swapchain{};

		std::array<framebuffer::unique_ptr, 3> world_framebuffers_{};
		renderpass::shared_ptr world_renderpass{};
		renderpass::shared_ptr ui_renderpass{};

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

		shader::shared_ptr material_shader{};

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



	static inline swapchain_support_details query_swapchain_support(vk::SurfaceKHR surface, const vk::PhysicalDevice& physical_device)
	{
		swapchain_support_details details;
		details.capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
		details.formats = physical_device.getSurfaceFormatsKHR(surface);
		details.present_modes = physical_device.getSurfacePresentModesKHR(surface);

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

	template<class T>
	consteval static vk::VertexInputBindingDescription get_binding_description() noexcept
	{
		vk::VertexInputBindingDescription bindingDescription{};
		bindingDescription
			.setBinding(0)
			.setStride(sizeof(T))
			.setInputRate(vk::VertexInputRate::eVertex);
		return bindingDescription;
	}

	consteval static std::array<vk::VertexInputAttributeDescription, 2> get_attribute_description() noexcept
	{
		std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].setBinding(0);
		attributeDescriptions[0].setLocation(0);
		attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);
		attributeDescriptions[0].setOffset(offsetof(vertex_3d, position));

		attributeDescriptions[1].setBinding(0);
		attributeDescriptions[1].setLocation(1);
		attributeDescriptions[1].setFormat(vk::Format::eR32G32Sfloat);
		attributeDescriptions[1].setOffset(offsetof(vertex_3d, tex));

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


}
