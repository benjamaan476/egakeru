#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>

#include "swapchain.h"
#include "vulkan_renderpass.h"
#include "command_buffer.h"
#include "fence.h"
#include "vulkan_renderbuffer.h"

#include "renderer/vertex_types.h"

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
		uint32_t graphics_index{ invalid_32_id };
		uint32_t present_index{ invalid_32_id };
		uint32_t compute_index{ invalid_32_id };
		uint32_t transfer_index{ invalid_32_id };
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
		uint8_t depth_channel_count{};

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

		uint32_t find_memory_index(uint32_t type_filter, vk::MemoryPropertyFlags property_flags) const;

		static bool physical_device_meets_requirements(vk::PhysicalDevice device, vk::SurfaceKHR surface, const vk::PhysicalDeviceProperties& properties, const vk::PhysicalDeviceFeatures features, const physical_device_requirements& requirements, physical_device_queue_family_info& family_info, swapchain_support_details& swapchain_support);
		static bool select_physical_device(vulkan_context* context);
		bool create(vulkan_context* context);
	};


	struct vulkan_context
	{
		vk::Instance instance;
		vk::AllocationCallbacks* allocator = nullptr;
		VkDebugUtilsMessengerEXT debug{};
		vulkan_device device{};
		vk::SurfaceKHR surface;
		swapchain::shared_ptr swpchain;

		egkr::vector<command_buffer> graphics_command_buffers;

		egkr::vector<vk::Semaphore> image_available_semaphore;
		egkr::vector<vk::Semaphore> queue_complete_semaphore;

		egkr::vector<fence::shared_ptr> in_flight_fences;
		egkr::vector<fence::shared_ptr> images_in_flight;

		uint32_t framebuffer_width{};
		uint32_t framebuffer_height{};
		uint32_t image_index{};
		uint32_t current_frame{};

		uint32_t framebuffer_size_generation{};
		uint32_t framebuffer_last_size_generation{};

		uint64_t geometry_vertex_offset{};
		uint64_t geometry_index_offset{};

		bool multithreading_enabled{};
		bool recreating_swapchain{};

#ifdef ENABLE_DEBUG_MACRO
		PFN_vkSetDebugUtilsObjectNameEXT pfn_set_debug_name{};
#endif

		float4 scissor_rect{};
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
		const std::array<uint8_t, 3> depth_counts{ 4,4,3 };
		auto flags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

		for (auto i{ 0U }; i < depth_formats.size(); ++i)
		{
			const auto& format = depth_formats[i];
			const auto properties = device.physical_device.getFormatProperties(format);

			if ((properties.linearTilingFeatures & flags) == flags)
			{
				device.depth_format = format;
				device.depth_channel_count = depth_counts[i];
				return true;
			}
			else if ((properties.optimalTilingFeatures & flags) == flags)
			{
				device.depth_format = format;
				device.depth_channel_count = depth_counts[i];
				return true;
			}
		}

		return false;
	}

	static inline queue_family_indices find_queue_families(const vulkan_context& context, const vk::PhysicalDevice& physical_device)
	{
		queue_family_indices queueIndices;
		auto familyProperties = physical_device.getQueueFamilyProperties();
		uint32_t index = 0;
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
	constexpr static vk::VertexInputBindingDescription get_binding_description() noexcept
	{
		vk::VertexInputBindingDescription bindingDescription{};
		bindingDescription
			.setBinding(0)
			.setStride(sizeof(T))
			.setInputRate(vk::VertexInputRate::eVertex);
		return bindingDescription;
	}

	constexpr static egkr::vector<vk::VertexInputAttributeDescription> get_3d_attribute_description() noexcept
	{
		egkr::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};

		vk::VertexInputAttributeDescription position{};
		position
			.setBinding(0)
			.setLocation(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(offsetof(vertex_3d, position));

		attributeDescriptions.push_back(position);

		vk::VertexInputAttributeDescription texture{};
		texture
			.setBinding(0)
			.setLocation(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(offsetof(vertex_3d, tex));

		attributeDescriptions.push_back(texture);

		return attributeDescriptions;
	}

	constexpr static egkr::vector<vk::VertexInputAttributeDescription> get_2d_attribute_description() noexcept
	{
		egkr::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};

		vk::VertexInputAttributeDescription position{};
		position
			.setBinding(0)
			.setLocation(0)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(offsetof(vertex_2d, position));

		attributeDescriptions.push_back(position);

		vk::VertexInputAttributeDescription texture{};
		texture
			.setBinding(0)
			.setLocation(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(offsetof(vertex_2d, tex));

		attributeDescriptions.push_back(texture);

		return attributeDescriptions;
	}

}
