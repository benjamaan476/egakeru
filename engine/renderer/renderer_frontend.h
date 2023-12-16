#pragma once
#include "pch.h"
#include "renderer_types.h"
#include "renderpass.h"
#include "event.h"
#include "renderer/camera.h"

#include "resources/material.h"
#include "resources/texture.h"
#include "resources/shader.h"

namespace egkr
{

	class renderer_frontend
	{
	public:
		using unique_ptr = std::unique_ptr<renderer_frontend>;
		API static unique_ptr create(backend_type type, const platform::shared_ptr& platform);

		renderer_frontend(backend_type type, const platform::shared_ptr& platform);

		API bool init();
		API void shutdown();
		API void on_resize(uint32_t width, uint32_t height);
		API void draw_frame(const render_packet& packet);

		void free_material(material* texture) const;

		renderpass::renderpass* get_renderpass(std::string_view renderpass_name) const;

		void regenerate_render_targets();
		const auto& get_backend() const { return backend_; }
	private:
		renderer_backend::unique_ptr backend_{};
		uint8_t window_attachment_count{};
		renderpass::renderpass* world_renderpass_{};
		egkr::vector<render_target::render_target::shared_ptr> world_render_targets_{3};
		egkr::vector<render_target::render_target::shared_ptr> ui_render_targets_{3};
		renderpass::renderpass* ui_renderpass_{};

		uint32_t framebuffer_width_{};
		uint32_t framebuffer_height_{};
		uint32_t material_shader_id{};
		uint32_t ui_shader_id{};
		//TODO temp
	};
}