#pragma once
#include "pch.h"
#include "renderer_types.h"
#include "event.h"

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

		//TODO Hack
		API void set_view(const float4x4& view, const float3& camera_position);

		void free_material(material* texture) const;

		bool populate_texture(texture* texture, const texture_properties& properties, const uint8_t* data) const;
		void free_texture(texture* texture) const;

		bool populate_geometry(geometry* geometry, const geometry_properties& properties) const;
		void free_geometry(geometry* geometry) const;

		bool populate_shader(shader* shader, uint32_t renderpass_id, const egkr::vector<std::string>& stage_filenames, const egkr::vector<shader_stages>& shader_stages) const;
		void free_shader(shader* shader) const;

		bool use_shader(shader* shader) const;
		bool bind_shader_globals(shader* shader) const;
		bool bind_shader_instances(shader* shader, uint32_t instance_id) const;
		bool apply_shader_globals(shader* shader) const;
		bool apply_shader_instances(shader* shader) const;
		uint32_t acquire_shader_isntance_resources(shader* shader) const;

		bool set_uniform(shader* shader, const shader_uniform& uniform, const void* value) const;

		builtin_renderpass get_renderpass_id(std::string_view renderpass_name) const;

		static bool on_event(event_code code, void* sender, void* listener, const event_context& context);

	private:
		renderer_backend::unique_ptr backend_{};

		float near_clip_{0.1F};
		float far_clip_{ 1000.F };
		float4x4 world_projection_{1.F};
		float4x4 world_view_{1.F};
		float3 camera_position_{};
		float4x4 ui_projection_{1.F};
		float4x4 ui_view_{1.F};
		uint32_t material_shader_id{};
		uint32_t ui_shader_id{};
		float4 ambient_colour_{ 0.25F, 0.25F, 0.25F, 1.F };
		uint32_t mode_{};
		//TODO temp
	};
}