#pragma once

#include "resource_loader.h"
#include <resources/material.h>

namespace egkr
{
	class material_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<material_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit material_loader(const loader_properties& properties);

		resource::shared_ptr load(std::string_view name, void* params) override;
		bool unload(const resource::shared_ptr& resource) override;

	private:
		static material_properties load_configuration_file(std::string_view path);

	};
}