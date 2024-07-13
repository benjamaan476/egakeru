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

		bool init(const configuration& configuration, uint8_t& out_window_attachment_count) final;
		void shutdown() final;
		void tidy_up() final;
		void resize(uint32_t width_, uint32_t height_) final;
		bool begin_frame() final;
		void end_frame() final;

		void free_material(material* texture) const override;

		texture* create_texture() const override;
		texture* create_texture(const texture::properties& properties, const uint8_t* data) const override;
		void create_texture(const texture::properties& properties, const uint8_t* data, texture* out_texture) const override;
		shader::shared_ptr create_shader(const shader::properties& properties) const override;
		geometry::shared_ptr create_geometry(const geometry::properties& properties) const override;
		render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height) const override;
		render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment_configuration>& attachments) const override;
		renderpass::renderpass::shared_ptr create_renderpass(const renderpass::configuration& configuration) const override;
		texture_map::shared_ptr create_texture_map(const texture_map::properties& properties) const override;
		renderbuffer::renderbuffer::shared_ptr create_renderbuffer(renderbuffer::type buffer_type, uint64_t size) const override;

		void set_viewport(const float4& rect) const override;
		void reset_viewport() const override;
		void set_scissor(const float4& rect) const override;
		void reset_scissor() const override;

		texture* get_window_attachment(uint8_t index) const override;
		texture* get_depth_attachment(uint8_t index) const override;
		uint8_t get_window_index() const override;

#ifdef ENABLE_DEBUG_MACRO
		static bool set_debug_obj_name(const vulkan_context* context, VkObjectType type, uint64_t handle, const std::string& name);
#define SET_DEBUG_NAME(context, type, handle, name) renderer_vulkan::set_debug_obj_name(context, type, handle, name);
#else
#define SET_DEBUG_NAME(type, handle, name)
#endif // ENABLE_DEBUG_MACRO

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

		bool is_multithreaded() const override;

	private:
		vulkan_context context_{};

		platform::shared_ptr platform_{};

#ifdef NDEBUG
		const bool enable_validation_layers_ = true;
#else
		const bool enable_validation_layers_ = true;
#endif
		std::string application_name_{};

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
