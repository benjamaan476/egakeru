#pragma once

#include "resource_loader.h"

namespace egkr
{
	class image_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<image_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit image_loader(const loader_properties& properties);

		//Params is a image_resource_parameters
		resource::shared_ptr load(std::string_view name, void* params) override;
		bool unload(const resource::shared_ptr& resource) override;
	};
}
