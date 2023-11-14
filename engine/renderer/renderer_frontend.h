#pragma once
#include "pch.h"
#include "renderer_types.h"
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

		bool populate_shader(shader::shader* shader, renderpass::renderpass* renderpass, const egkr::vector<std::string>& stage_filenames, const egkr::vector<shader::stages>& shader_stages) const;
		void free_shader(shader::shader* shader) const;

		bool use_shader(shader::shader* shader) const;
		bool bind_shader_globals(shader::shader* shader) const;
		bool bind_shader_instances(shader::shader* shader, uint32_t instance_id) const;
		bool apply_shader_globals(shader::shader* shader) const;
		bool apply_shader_instances(shader::shader* shader, bool needs_update) const;
		uint32_t acquire_shader_isntance_resources(shader::shader* shader, const egkr::vector<texture::texture_map*>& texture_maps) const;

		void acquire_texture_map(texture::texture_map* map) const;
		void release_texture_map(texture::texture_map* map) const;

		bool set_uniform(shader::shader* shader, const shader::uniform& uniform, const void* value) const;

		renderpass::renderpass* get_renderpass(std::string_view renderpass_name) const;
		void populate_render_target(render_target::render_target* render_target, const egkr::vector<texture::texture::shared_ptr>& attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height) const;
		void free_render_target(render_target::render_target* render_target, bool free_internal_memory) const;

		static bool on_event(event_code code, void* sender, void* listener, const event_context& context);

		void regenerate_render_targets();
		const auto& get_backend() const { return backend_; }
	private:
		renderer_backend::unique_ptr backend_{};
		uint8_t window_attachment_count{};
		renderpass::renderpass* world_renderpass_{};
		egkr::vector<render_target::render_target> world_render_targets_{3};
		egkr::vector<render_target::render_target> ui_render_targets_{3};
		renderpass::renderpass* ui_renderpass_{};

		uint32_t framebuffer_width_{};
		uint32_t framebuffer_height_{};
		camera::shared_ptr world_camera_{};
		float near_clip_{0.1F};
		float far_clip_{ 1000.F };
		float4x4 world_projection_{1.F};
		float4x4 ui_projection_{1.F};
		float4x4 ui_view_{1.F};
		uint32_t material_shader_id{};
		uint32_t ui_shader_id{};
		float4 ambient_colour_{ 0.25F, 0.25F, 0.25F, 1.F };
		uint32_t mode_{};
		//TODO temp
	};
}