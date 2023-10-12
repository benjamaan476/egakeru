#pragma once

#include "pch.h"

#include "resources/material.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
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

		static material::shared_ptr acquire(std::string_view name);
		static material::shared_ptr acquire(const material_properties& properties);
	private:
		static bool create_default_material();
		static bool load_material(const material_properties& properties, material::shared_ptr& material);
	private:
		const renderer_frontend* renderer_{};
		uint32_t max_material_count_{};

		material::shared_ptr default_material_{};
		egkr::vector<material::shared_ptr> registered_materials_{};
		std::unordered_map<std::string, material_reference> registered_materials_by_name_{};

	};
}
