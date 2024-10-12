#pragma once

#include "pch.h"

#include <systems/system.h>
#include "loaders/resource_loader.h"

namespace egkr
{
	class resource_system : public system
	{
	public:
		struct configuration
		{
			uint32_t max_loader_count{};
			std::string base_path{};
		};
		using unique_ptr = std::unique_ptr<resource_system>;
		static resource_system* create(const configuration& properties);

		explicit resource_system(const configuration& properties);
		~resource_system() override;

		bool init() override;
		bool shutdown() override;

		void register_loader(resource_loader::unique_ptr loader);

		static resource::shared_ptr load(const std::string& name, resource::type type, void* params);
		static bool unload(const resource::shared_ptr& resource);
	private:
		uint32_t max_loader_count_{};
		std::string base_path_{};

		std::unordered_map<resource::type, resource_loader::unique_ptr> registered_loaders_{};
	};
}
