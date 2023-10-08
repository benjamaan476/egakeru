#include "resource_system.h"

#include "loaders/image_loader.h"
#include "loaders/material_loader.h"

namespace egkr
{
	static resource_system::unique_ptr resource_system_{};

	bool resource_system::create(const resource_system_configuration& properties)
	{
		resource_system_ = std::make_unique<resource_system>(properties);
		return resource_system_->init();
	}

	resource_system::resource_system(const resource_system_configuration& properties)
		: max_loader_count_{ properties.max_loader_count }, base_path_{ properties.base_path }
	{
	}

	resource_system::~resource_system()
	{
		shutdown();
	}

	bool resource_system::init()
	{
		if (resource_system_->max_loader_count_ == 0)
		{
			LOG_FATAL("Max loaders must be non-zero");
			return false;
		}

		resource_system_->registered_loaders_.reserve(resource_system_->max_loader_count_);

			//Register known loaders
		{
			loader_properties image_loader_properties{};
			image_loader_properties.custom_type = {};
			image_loader_properties.path = "../assets/textures";

			register_loader(image_loader::create(image_loader_properties));
		}
		{
			loader_properties material_loader_properties{};
			material_loader_properties.custom_type = {};
			material_loader_properties.path = "../assets/materials";

			register_loader(material_loader::create(material_loader_properties));
		}
		return true;
	}

	void resource_system::shutdown()
	{
		resource_system_->registered_loaders_.clear();
	}

	void resource_system::register_loader(resource_loader::unique_ptr loader)
	{
		if (resource_system_->registered_loaders_.contains(loader->get_loader_type()))
		{
			LOG_WARN("Already registered loaded of this type. Doing nothing");
			return;
		}

		resource_system_->registered_loaders_[loader->get_loader_type()] = std::move(loader);
	}

	resource::shared_ptr resource_system::load(std::string_view name, resource_type type)
	{
		if (resource_system_->registered_loaders_.contains(type))
		{
			return resource_system_->registered_loaders_[type]->load(name);
		}

		LOG_ERROR("Attempted to load resource without corresponding loader registered");
		return nullptr;
	}

	bool resource_system::unload(const resource::shared_ptr& resource)
	{
		if (resource_system_->registered_loaders_.contains(resource->get_type()))
		{
			return resource_system_->registered_loaders_[resource->get_type()]->unload(resource);
		}

		LOG_ERROR("Tried to unload a resource without a corresponding loader registered");
		return false;
	}
}