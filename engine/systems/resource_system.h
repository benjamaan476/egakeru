#pragma once

#include "pch.h"

#include <systems/system.h>
#include "loaders/resource_loader.h"

namespace egkr
{

	struct resource_system_configuration
	{
		uint32_t max_loader_count{};
		std::string base_path{};
	};

	class resource_system : public system
	{
	public:
		using unique_ptr = std::unique_ptr<resource_system>;
		static resource_system* create(const resource_system_configuration& properties);

		explicit resource_system(const resource_system_configuration& properties);
		~resource_system();

		bool init() override;
		bool update(float delta_time) override;
		bool shutdown() override;

		void register_loader(resource_loader::unique_ptr loader);

		static resource::shared_ptr load(std::string_view name, resource_type type, void* params);
		static bool unload(const resource::shared_ptr& resource);
	private:
		uint32_t max_loader_count_{};
		std::string base_path_{};

		std::unordered_map<resource_type, resource_loader::unique_ptr> registered_loaders_{};
	};
}
