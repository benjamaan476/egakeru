#pragma once

#include "pch.h"

#include "../renderer_types.h"
#include "vulkan_types.h"

namespace egkr
{
	class swapchain;
	class renderer_vulkan : public renderer_backend
	{
	public:
		static renderer_backend::unique_ptr create(const platform::shared_ptr& platform);

		explicit renderer_vulkan(platform::shared_ptr platform);
		~renderer_vulkan();

		bool init() final;
		void shutdown() final;
		void resize(uint32_t width_, uint32_t height_) final;
		bool begin_frame(std::chrono::milliseconds delta_time) final;
		void end_frame() final;
	private:
		bool init_instance();
		bool create_debug_messenger();

		vk::SurfaceKHR create_surface();

		bool pick_physical_device();
		bool is_device_suitable(const vk::PhysicalDevice& device);
		static bool check_device_extension_support(const vk::PhysicalDevice& physical_device);

		bool create_logical_device();
		void create_command_buffers();

		bool recreate_swapchain();

		void create_object_shader();
		void create_object_buffers();


		void upload_data_range(vk::CommandPool pool, vk::Fence fence, vk::Queue queue, buffer::shared_ptr buffer, uint64_t offset, uint64_t size, const void* data);
	private:
		vulkan_context context_{};

		platform::shared_ptr platform_{};
		uint32_t frame_number_{};

#ifdef NDEBUG
		const bool enable_validation_layers_ = false;
#else
		const bool enable_validation_layers_ = true;
#endif

		static const inline std::vector<const char*> validation_layers_
		{
			"VK_LAYER_KHRONOS_validation",
		};

		static const inline std::vector<const char*> device_extensions_
		{
			"VK_KHR_swapchain"
		};


	};
}