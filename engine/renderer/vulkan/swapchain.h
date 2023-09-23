#pragma once

#include "pch.h"
#include "image.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct vulkan_context;
	class swapchain
	{
	public:
		using unique_ptr = std::unique_ptr<swapchain>;

		static unique_ptr create(vulkan_context* context);

		explicit swapchain(vulkan_context* context);
		~swapchain();

		void destroy();

		void recreate();
		uint32_t acquire_next_image_index(vk::Semaphore semaphore, vk::Fence fence);
		void present(vk::Queue graphics_queue, vk::Queue present_queue, vk::Semaphore render_complete, uint32_t image_index);

	private:

		void create();
		static vk::SurfaceFormatKHR choose_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats);
		static vk::PresentModeKHR choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes);
		vk::Extent2D choose_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities);
		std::vector<vk::Image> get_swapchain_images(vk::SwapchainKHR swapchain);

	private:
		vulkan_context* context_;
		vk::SurfaceFormatKHR format_{};
		vk::Extent2D extent_{};
		vk::SwapchainKHR swapchain_{};

		egkr::vector<vk::Image> images_{};
		egkr::vector<vk::ImageView> image_views_{};
		vulkan_image::shared_ptr depth_attachment_{};

		uint32_t image_count_{};
		uint8_t frames_in_flight_{3};

	};
}