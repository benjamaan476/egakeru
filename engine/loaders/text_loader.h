#pragma once

#include "resource_loader.h"

namespace egkr
{
	class text_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<text_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit text_loader(const loader_properties& properties);
		~text_loader() override = default;

		resource::shared_ptr load(std::string_view name, void* params) override;
		bool unload(const resource::shared_ptr& resource) override;
	};
}
