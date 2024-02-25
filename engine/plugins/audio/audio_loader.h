#pragma once
#include <pch.h>

#include <loaders/resource_loader.h>

namespace egkr
{


	class audio_loader : public resource_loader
	{
	public:
		using unique_ptr = std::unique_ptr<audio_loader>;
		static unique_ptr create(const loader_properties& properties);

		explicit audio_loader(const loader_properties& properties);
		~audio_loader() override = default;

		resource::shared_ptr load(std::string_view name, void* params) override;
		bool unload(const resource::shared_ptr& resource) override;
	};
}
