#pragma once

#include "pch.h"
#include "image.h"
#include "resources/texture.h"
#include "renderer/renderer_types.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct vulkan_context;
	class swapchain
	{
	public:
		using shared_ptr = std::shared_ptr<swapchain>;

		static shared_ptr create(vulkan_context* context);

		explicit swapchain(vulkan_context* context);
		~swapchain();

		void destroy();

		void recreate();
		uint32_t acquire_next_image_index(vk::Semaphore semaphore, vk::Fence fence);
		void present(vk::Queue graphics_queue, vk::Queue present_queue, vk::Semaphore render_complete, uint32_t image_index);

		const auto& get_format() const { return format_; }
		const auto& get_image_count() const { return image_count_; }
		const auto& get_depth_attachment() const { return depth_attachment_; }
		const auto& get_max_frames_in_flight() const { return max_frames_in_flight_; }

		const auto& get_render_texture(uint32_t frame) const { return render_textures_[frame]; }
		const auto& get_framebuffer(uint32_t frame) const { return render_targets_[frame]; }
		auto& get_render_targets() const { return render_targets_; }
	private:

		void create();
		static vk::SurfaceFormatKHR choose_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats);
		static vk::PresentModeKHR choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes);
		vk::Extent2D choose_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities);
		std::vector<vk::Image> get_swapchain_images(vk::SwapchainKHR swapchain);

	private:
		vulkan_context* context_;
		egkr::vector<render_target::render_target::shared_ptr> render_targets_{};

		vk::SurfaceFormatKHR format_{};
		vk::Extent2D extent_{};
		vk::SwapchainKHR swapchain_{};

		egkr::vector<texture::texture::shared_ptr> render_textures_{};
		texture::texture::shared_ptr depth_attachment_{};

		uint32_t image_count_{};
		uint8_t max_frames_in_flight_{3};

	};
}