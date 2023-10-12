#include "material_system.h"

#include "texture_system.h"
#include "platform/filesystem.h"
#include "systems/resource_system.h"

namespace egkr
{
	static material_system::unique_ptr material_system_{};

	bool material_system::create(const renderer_frontend* renderer)
	{
		material_system_ = std::make_unique<material_system>(renderer);
		return material_system::init();
	}

	material_system::material_system(const renderer_frontend* renderer)
		: renderer_{ renderer }, max_material_count_{ 1024 }
	{
	}

	material_system::~material_system()
	{
		shutdown();
	}

	bool material_system::init()
	{
		if (material_system_->max_material_count_ == 0)
		{
			LOG_FATAL("Material max count must be > 0");
			return false;
		}

		material_system_->registered_materials_.reserve(material_system_->max_material_count_);
		material_system_->registered_materials_by_name_.reserve(material_system_->max_material_count_);

		if (!create_default_material())
		{
			LOG_FATAL("Failed to create default material");
			return false;
		}


		return true;
	}

	void material_system::shutdown()
	{
		if (material_system_->default_material_)
		{
			material_system_->renderer_->free_material(material_system_->default_material_.get());
			material_system_->default_material_.reset();
		}

		for (auto& material : material_system_->registered_materials_)
		{
			material_system_->renderer_->free_material(material.get());
		}
		material_system_->registered_materials_.clear();

		material_system_->registered_materials_by_name_.clear();
	}

	material::shared_ptr material_system::acquire(std::string_view name)
	{
		auto material_resource = resource_system::load(name, resource_type::material);

		auto* material_config = (material_properties*)material_resource->data;

		auto material = acquire(*material_config);

		resource_system::unload(material_resource);

		return material;
	}

	material::shared_ptr material_system::acquire(const material_properties& properties)
	{
		if (strcmp(properties.name.data(), default_material_name_.data()) == 0)
		{
			return material_system_->default_material_;
		}


		if (material_system_->registered_materials_by_name_.contains(properties.name.data()))
		{
			auto material_handle = material_system_->registered_materials_by_name_[properties.name.data()];
			return material_system_->registered_materials_[material_handle];
		}

		uint32_t material_id = material_system_->registered_materials_.size();

		if (material_id >= material_system_->max_material_count_)
		{
			LOG_FATAL("Exceeded max texture count");
			return nullptr;
		}

		auto new_material = material::create(material_system_->renderer_, properties);
		load_material(properties, new_material);

		//if (new_material->get_generation() == invalid_id)
		//{
		//	new_material->set_generation(0);
		//}
		//else
		//{
		//	new_material->increment_generation();
		//}

		material_system_->registered_materials_.push_back(new_material);
		material_system_->registered_materials_by_name_[properties.name.data()] = material_id;
		return new_material;
	}

	bool material_system::create_default_material()
	{
		material_properties properties{};
		properties.name = "default";
		properties.type = material_type::world;
		properties.diffuse_map_name = "default_texture";
		properties.diffuse_colour = float4{ 1.F };
		material_system_->default_material_ = material::create(material_system_->renderer_, properties);
		return true;
	}

	bool material_system::load_material(const material_properties& properties, material::shared_ptr& material)
	{

		//Get diffuse map
		auto texture = texture_system::acquire(properties.diffuse_map_name);
		if (texture == nullptr)
		{
			LOG_WARN("Failed to find texture: {} for material {}. Setting default", properties.diffuse_map_name.data(), properties.name.data());
			texture = texture_system::get_default_texture();
		}


			auto temp_material = material::create(material_system_->renderer_, properties);

			material.reset();
			material = std::move(temp_material);
			if (material->get_generation() == invalid_id)
			{
				material->set_generation(0);
			}
			else
			{
				material->increment_generation();
			}
		material->set_diffuse_colour(properties.diffuse_colour);
		//material->set_name(properties.name);
		material->set_diffuse_map({ std::move(texture),texture_use::map_diffuse });

		return true;
	}
}