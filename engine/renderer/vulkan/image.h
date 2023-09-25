#pragma once

#include "pch.h"
#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct vulkan_context;
	struct image_properties
	{
		vk::Format image_format{};
		vk::ImageTiling tiling{};
		vk::ImageUsageFlags usage{};
		vk::MemoryPropertyFlags memory_properties{};
		vk::ImageAspectFlags aspect_flags{};
		
		//TODO: support configurable depth, mip levels, layers, sample count, sharing mode
		const vk::SampleCountFlagBits samples{vk::SampleCountFlagBits::e1};
		const vk::SharingMode sharing_mode{vk::SharingMode::eExclusive};

		const uint32_t depth{ 1 };
		const uint32_t mip_levels{ 4 };
		const uint32_t array_layers{ 1 };
	};

	class vulkan_image : std::enable_shared_from_this<vulkan_image>
	{
	public:
		using shared_ptr = std::shared_ptr<vulkan_image>;
		API static shared_ptr create(const vulkan_context* context, uint32_t width_, uint32_t height_, const image_properties& properties, bool create_view);
		void create_view(const image_properties& properties);

		vulkan_image(const vulkan_context* context, uint32_t width_, uint32_t height_, const image_properties& properties);
		~vulkan_image();

		void destroy();

		const auto& get_view() const { return view_; }

	private:
		const vulkan_context* context_{};
		vk::Image image_{};
		vk::DeviceMemory memory_{};
		vk::ImageView view_{};
		uint32_t width_{};
		uint32_t height_{};
	};
}