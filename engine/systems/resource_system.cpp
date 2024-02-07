#include "resource_system.h"

#include "loaders/image_loader.h"
#include "loaders/material_loader.h"
#include "loaders/binary_loader.h"
#include "loaders/text_loader.h"
#include "loaders/shader_loader.h"
#include "loaders/bitmap_font_loader.h"
#include "loaders/mesh_loader.h"

namespace egkr
{
	static resource_system::unique_ptr resource_system_{};

	bool resource_system::create(const resource_system_configuration& properties)
	{
		resource_system_ = std::make_unique<resource_system>(properties);
		return true;
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

		auto base_path = resource_system_->base_path_;
			//Register known loaders
		{
			loader_properties image_loader_properties{};
			image_loader_properties.custom_type = {};
			image_loader_properties.path = base_path + "textures";

			register_loader(image_loader::create(image_loader_properties));
		}
		{
			loader_properties material_loader_properties{};
			material_loader_properties.custom_type = {};
			material_loader_properties.path = base_path + "materials";

			register_loader(material_loader::create(material_loader_properties));
		}
		{
			loader_properties binary_loader_properties{};
			binary_loader_properties.custom_type = {};
			binary_loader_properties.path = base_path;

			register_loader(binary_loader::create(binary_loader_properties));
		}
		{
			loader_properties text_loader_properties{};
			text_loader_properties.custom_type = {};
			text_loader_properties.path = base_path;

			register_loader(text_loader::create(text_loader_properties));
		}
		{
			loader_properties shader_loader_properties{};
			shader_loader_properties.custom_type = {};
			shader_loader_properties.path = base_path + "shaders";

			register_loader(shader_loader::create(shader_loader_properties));
		}
		{
			loader_properties mesh_loader_properties{};
			mesh_loader_properties.custom_type = {};
			mesh_loader_properties.path = base_path + "meshes";

			register_loader(mesh_loader::create(mesh_loader_properties));
		}
		{
			loader_properties bitmap_font_loader{ .path = base_path + "fonts", .custom_type = {} };
			register_loader(bitmap_font_loader::create(bitmap_font_loader));
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

	resource::shared_ptr resource_system::load(std::string_view name, resource_type type, void* params)
	{
		if (resource_system_->registered_loaders_.contains(type))
		{
			return resource_system_->registered_loaders_[type]->load(name, params);
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