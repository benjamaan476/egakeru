#pragma once

#include "resource_loader.h"

namespace egkr
{
	class scene_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<scene_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit scene_loader(const loader_properties& properties);
		~scene_loader() override = default;

		resource::shared_ptr load(std::string_view name, void* params) override;
		bool unload(const resource::shared_ptr& resource) override;
	};
}
