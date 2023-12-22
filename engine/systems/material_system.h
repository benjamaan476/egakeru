#pragma once

#include "pch.h"

#include "resources/material.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
	struct material_shader_uniform_location
	{
		uint16_t projection{};
		uint16_t view{};
		uint16_t ambient_colour{};
		uint16_t view_position{};
		uint16_t diffuse_colour{};
		uint16_t diffuse_texture{};
		uint16_t specular_texture{};
		uint16_t normal_texture{};
		uint16_t shininess{};
		uint16_t model{};
		uint16_t mode{};
		uint16_t directional_light{};
	};

	struct ui_shader_uniform_location
	{
		uint16_t projection{};
		uint16_t view{};
		uint16_t diffuse_colour{};
		uint16_t diffuse_texture{};
		uint16_t model{};
	};

	class material_system
	{
	public:
		using material_reference = uint32_t;
		using unique_ptr = std::unique_ptr<material_system>;
		static bool create(const renderer_frontend* renderer);

		material_system(const renderer_frontend* renderer);
		~material_system();

		static bool init();
		static void shutdown();

		static material::shared_ptr get_default_material();
		static material::shared_ptr acquire(std::string_view name);
		static material::shared_ptr acquire(const material_properties& properties);

		static void apply_global(uint32_t shader_id, const float4x4& projection, const float4x4& view, const float4& ambient_colour, const float3& view_position, uint32_t mode);
		static void apply_instance(const material::shared_ptr& material, bool needs_update);
		static void apply_local(const material::shared_ptr& material, const float4x4& model);
	private:
		static bool create_default_material();
		static bool load_material(const material_properties& properties, material::shared_ptr& material);
	private:
		const renderer_frontend* renderer_{};
		uint32_t max_material_count_{};

		material::shared_ptr default_material_{};
		egkr::vector<material::shared_ptr> registered_materials_{};
		std::unordered_map<std::string, material_reference> registered_materials_by_name_{};

		material_shader_uniform_location material_locations_{};
		ui_shader_uniform_location ui_locations_{};

		uint32_t material_shader_id_{invalid_32_id};
		uint32_t ui_shader_id_{invalid_32_id};

	};
}
