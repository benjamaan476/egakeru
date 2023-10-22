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
		~renderer_vulkan();

		bool init() final;
		void shutdown() final;
		void resize(uint32_t width_, uint32_t height_) final;
		bool begin_frame() final;
		bool begin_renderpass(builtin_renderpass renderpass) final;
		bool end_renderpass(builtin_renderpass renderpass) final;
		void draw_geometry(const geometry_render_data& model) final;
		void end_frame() final;

		const void* get_context() const final { return &context_; }

		void free_material(material* texture) override;

		bool populate_texture(texture* texture, const texture_properties& properties, const uint8_t* data) override;
		bool populate_writeable_texture(texture* texture) override;
		bool resize_texture(texture* texture, uint32_t width, uint32_t height) override;
		bool texture_write_data(texture* texture, uint64_t offset, uint32_t size, const uint8_t* data) override;
		void free_texture(texture* texture) override;

		bool populate_geometry(geometry* geometry, const geometry_properties& properties) override;
		void free_geometry(geometry* geometry) override;

		bool populate_shader(shader* shader, uint32_t renderpass_id, const egkr::vector<std::string>& stage_filenames, const egkr::vector<shader_stages>& shader_stages) override;
		void free_shader(shader* shader) override;

		bool use_shader(shader* shader) override;
		bool bind_shader_globals(shader* shader) override;
		bool bind_shader_instances(shader* shader, uint32_t instance_id) override;
		bool apply_shader_globals(shader* shader) override;
		bool apply_shader_instances(shader* shader, bool needs_update) override;
		uint32_t acquire_shader_isntance_resources(shader* shader, const egkr::vector<texture_map*>& texture_maps) override;
		void acquire_texture_map(texture_map* map) override;
		void release_texture_map(texture_map* map) override;

		bool set_uniform(shader* shader, const shader_uniform& uniform, const void* value) override;
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

		vulkan_shader_stage create_module(shader* shader, const vulkan_shader_stage_configuration& configuration);

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