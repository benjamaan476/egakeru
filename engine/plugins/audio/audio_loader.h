#pragma once
#include <loaders/resource_loader.h>

namespace egkr
{
	class audio_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<audio_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit audio_loader(const loader_properties& properties);

		resource::shared_ptr load(const std::string& name, void* params) override;
		bool unload(const resource::shared_ptr& resource) override;
	};
}
