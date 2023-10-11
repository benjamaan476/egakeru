#pragma once

#include "pch.h"

#include "resources/material.h"

namespace egkr
{
	struct material_shader_uniform_location
	{
		uint16_t projection{};
		uint16_t view{};
		uint16_t diffuse_colour{};
		uint16_t diffuse_texture{};
		uint16_t model{};
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
		static bool create(const void* renderer_context);

		material_system(const void* renderer_context);
		~material_system();

		static bool init();
		static void shutdown();

		static material::shared_ptr acquire(std::string_view name);
		static material::shared_ptr acquire(const material_properties& properties);

		static void apply_global(uint32_t shader_id, const float4x4& projection, const float4x4& view);
		static void apply_instance(const material::shared_ptr& material);
		static void apply_local(const material::shared_ptr& material, const float4x4& model);
	private:
		static bool create_default_material();
		static bool load_material(const material_properties& properties, material::shared_ptr& material);
	private:
		const void* renderer_context_{};
		uint32_t max_material_count_{};

		material::shared_ptr default_material_{};
		egkr::vector<material::shared_ptr> registered_materials_{};
		std::unordered_map<std::string, material_reference> registered_materials_by_name_{};

		material_shader_uniform_location material_locations_{};
		ui_shader_uniform_location ui_locations_{};

		uint32_t material_shader_id_{};
		uint32_t ui_shader_id_{};

	};
}
