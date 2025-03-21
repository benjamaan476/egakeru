#pragma once

#include "pch.h"

#include "renderer/renderer_types.h"
#include "vulkan_types.h"
#include "vulkan_shader.h"

namespace egkr
{
    class swapchain;
    class renderer_vulkan : public renderer_backend
    {
    public:
	static renderer_backend::unique_ptr create();

	explicit renderer_vulkan();
	~renderer_vulkan() override;

	bool init(const configuration& configuration, const platform::shared_ptr& platform, uint8_t& out_window_attachment_count) final;
	void shutdown() final;
	void tidy_up() final;
	void resize(uint32_t width_, uint32_t height_) final;

	bool prepare_frame(frame_data& frame_data) final;
	bool begin(const frame_data& frame_data) final;
	void end(frame_data& frame_data) final;
	void present(const frame_data& frame_data) final;

	[[nodiscard]] texture::shared_ptr create_texture() const override;
	texture::shared_ptr create_texture(const texture::properties& properties, const uint8_t* data) const override;
	void create_texture(const texture::properties& properties, const uint8_t* data, texture* out_texture) const override;
	[[nodiscard]] shader::shared_ptr create_shader(const shader::properties& properties) const override;
	[[nodiscard]] geometry::shared_ptr create_geometry(const geometry::properties& properties) const override;
	render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height) const override;
	[[nodiscard]] render_target::render_target::shared_ptr create_render_target(const egkr::vector<render_target::attachment_configuration>& attachments) const override;
	[[nodiscard]] renderpass::renderpass::shared_ptr create_renderpass(const renderpass::configuration& configuration) const override;
	[[nodiscard]] texture_map::shared_ptr create_texture_map(const texture_map::properties& properties) const override;
	[[nodiscard]] renderbuffer::renderbuffer::shared_ptr create_renderbuffer(renderbuffer::type buffer_type, uint64_t size) const override;

	void set_viewport(const float4& rect) const override;
	void set_scissor(const float4& rect) const override;
	void reset_scissor() const override;
	void set_winding(winding winding) const override;

	[[nodiscard]] uint32_t get_window_attachment_count() const override;
	[[nodiscard]] texture::shared_ptr get_window_attachment(uint8_t index) const override;
	[[nodiscard]] texture::shared_ptr get_depth_attachment(uint8_t index) const override;
	[[nodiscard]] uint8_t get_window_index() const override;

#ifdef ENABLE_DEBUG_MACRO
	static bool set_debug_obj_name(const vulkan_context* context, VkObjectType type, uint64_t handle, const std::string& name);
#define SET_DEBUG_NAME(context, type, handle, name) renderer_vulkan::set_debug_obj_name(context, type, handle, name);
#else
#define SET_DEBUG_NAME(context, type, handle, name)
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
	platform::shared_ptr platform_;

#ifdef NDEBUG
	constexpr static bool enable_validation_layers_ = false;
#else
	constexpr static bool enable_validation_layers_ = true;
#endif
	std::string application_name_;

	static const inline std::vector<const char*> validation_layers_{
	    "VK_LAYER_KHRONOS_validation",
	};

	static const inline std::vector<const char*> device_extensions_{"VK_KHR_swapchain"};
    };
}
