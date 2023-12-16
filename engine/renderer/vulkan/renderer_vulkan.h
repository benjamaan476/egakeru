#pragma once

#include "pch.h"

#include "../renderer_types.h"
#include "vulkan_types.h"
#include "vulkan_shader.h"

namespace egkr
{
	class swapchain;
	class renderer_vulkan : public renderer_backend
	{
	public:
		static renderer_backend::unique_ptr create(const platform::shared_ptr& platform);

		explicit renderer_vulkan(platform::shared_ptr platform);
		~renderer_vulkan() override;

		bool init(const renderer_backend_configuration& configuration, uint8_t& out_window_attachment_count) final;
		void shutdown() final;
		void resize(uint32_t width_, uint32_t height_) final;
		bool begin_frame() final;
		void end_frame() final;

		void free_material(material* texture) const override;

		texture::texture::shared_ptr create_texture(const texture::properties& properties, const uint8_t* data) const override;
		shader::shader::shared_ptr create_shader(const shader::properties& properties) const override;
		geometry::geometry::shared_ptr create_geometry(const geometry::properties& properties) const override;
		render_target::render_target::shared_ptr create_render_target() const override;
		texture_map::texture_map::shared_ptr create_texture_map(const texture_map::properties& properties) const override;

		texture::texture::shared_ptr get_window_attachment(uint8_t index)override;
		texture::texture::shared_ptr get_depth_attachment()override;
		uint8_t get_window_index()override;
		renderpass::renderpass* get_renderpass(std::string_view name) const override;
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

	private:
		vulkan_context context_{};

		platform::shared_ptr platform_{};

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