#pragma once

#include "pch.h"

#include "resources/material.h"

namespace egkr
{

	struct material_properties
	{
		float4 diffuse_colour{};
		std::string name{};
		std::string diffuse_map_name{};
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
	private:
		static bool create_default_material();
		static bool load_material(const material_properties& properties, material::shared_ptr& material);
		static material_properties load_configuration_file(std::string_view path);
	private:
		const void* renderer_context_{};
		uint32_t max_material_count_{};

		material::shared_ptr default_material_{};
		egkr::vector<material::shared_ptr> registered_materials_{};
		std::unordered_map<std::string, material_reference> registered_materials_by_name_{};

	};
}
