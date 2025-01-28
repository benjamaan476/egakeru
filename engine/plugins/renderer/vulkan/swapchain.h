#pragma once

#include "pch.h"
#include "vulkan_texture.h"
#include "resources/texture.h"
#include "renderer/renderer_types.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{
    struct vulkan_context;
    class swapchain
    {
    public:
	struct configuration
	{
	    renderer_backend::flags flags{};
	};

	using shared_ptr = std::shared_ptr<swapchain>;

	static shared_ptr create(vulkan_context* context, const configuration& configuration);

	explicit swapchain(vulkan_context* context, const configuration& configuration);
	~swapchain();

	void destroy();

	void recreate();
	uint32_t acquire_next_image_index(vk::Semaphore semaphore, vk::Fence fence);
	void present(vk::Queue graphics_queue, vk::Queue present_queue, vk::Semaphore render_complete, uint32_t image_index);

	[[nodiscard]] const auto& get_format() const { return format_; }
	[[nodiscard]] const auto& get_image_count() const { return image_count_; }
	[[nodiscard]] const auto& get_max_frames_in_flight() const { return max_frames_in_flight_; }

	[[nodiscard]] const auto& get_render_texture(uint32_t frame) const { return render_textures_[frame]; }
	[[nodiscard]] const auto& get_depth_attachment(uint32_t frame) const { return depth_attachments_[frame]; }
	[[nodiscard]] const auto& get_framebuffer(uint32_t frame) const { return render_targets_[frame]; }
	[[nodiscard]] auto& get_render_targets() const { return render_targets_; }
    private:
	void create();
	static vk::SurfaceFormatKHR choose_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats);
	[[nodiscard]] vk::PresentModeKHR choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes) const;
	vk::Extent2D choose_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities);
	std::vector<vk::Image> get_swapchain_images(vk::SwapchainKHR swapchain);
    private:
	vulkan_context* context_;
	configuration configuration_{};
	egkr::vector<render_target::render_target::shared_ptr> render_targets_;

	vk::SurfaceFormatKHR format_;
	vk::Extent2D extent_;
	vk::SwapchainKHR swapchain_;

	egkr::vector<texture::shared_ptr> render_textures_;
	egkr::vector<texture::shared_ptr> depth_attachments_;

	uint8_t image_count_{};
	uint8_t max_frames_in_flight_{3};
    };
}
