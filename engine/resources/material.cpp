#include "material.h"

#include "engine/engine.h"

#include "systems/texture_system.h"
#include "systems/shader_system.h"

namespace egkr
{
	material::shared_ptr material::create(const material_properties& properties)
	{
		auto mat = std::make_shared<material>(properties);
		return mat;
	}

	material::material(const material_properties& material_properties)
		: resource(0, 0, material_properties.name), diffuse_colour_{ material_properties.diffuse_colour }, shader_name_{ material_properties.shader_name }
	{
		diffuse_map_ = texture_map::create({});
		if (material_properties.diffuse_map_name == default_diffuse_name)
		{
			diffuse_map_->map_texture = texture_system::get_default_diffuse_texture();
		}
		else
		{
			diffuse_map_->map_texture = texture_system::acquire(material_properties.diffuse_map_name);
		}

		specular_map_ = texture_map::create({});
		if (material_properties.specular_map_name == default_specular_name)
		{
			specular_map_->map_texture = texture_system::get_default_specular_texture();
		}
		else
		{
			specular_map_->map_texture = texture_system::acquire(material_properties.specular_map_name);
		}

		normal_map_ = texture_map::create({});
		if (material_properties.normal_map_name == default_normal_name)
		{
			normal_map_->map_texture = texture_system::get_default_diffuse_texture();
		}
		else
		{
			normal_map_->map_texture = texture_system::acquire(material_properties.normal_map_name);
		}
		shader_id_ = shader_system::get_shader_id(shader_name_);

	}

	material::~material()
	{
		engine::get()->get_renderer()->free_material(this);
		diffuse_map_.reset();
		specular_map_.reset();
		normal_map_.reset();
	}

	void material::free()
	{
		diffuse_map_->release();
		specular_map_->release();
		normal_map_->release();
	}

	void material::set_diffuse_colour(const float4 diffuse)
	{
		diffuse_colour_ = diffuse;
	}
}
