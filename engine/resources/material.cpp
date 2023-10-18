#include "material.h"

#include "renderer/renderer_frontend.h"

#include "systems/texture_system.h"
#include "systems/shader_system.h"

namespace egkr
{
	material::shared_ptr material::create(const material_properties& properties)
	{
		auto mat = std::make_shared<material>(properties);
		return mat;
	}

	material::material(const material_properties& properties)
		: resource(0, 0, properties.name), diffuse_colour_{ properties.diffuse_colour }, shader_name_{ properties.shader_name }
	{

		if (properties.diffuse_map_name == default_diffuse_name)
		{
			diffuse_map_.texture = texture_system::get_default_diffuse_texture();
		}
		else
		{
			diffuse_map_.texture = texture_system::acquire(properties.diffuse_map_name);
		}

		if (properties.specular_map_name == default_specular_name)
		{
			specular_map_.texture = texture_system::get_default_specular_texture();
		}
		else
		{
			specular_map_.texture = texture_system::acquire(properties.specular_map_name);
		}

		if (properties.normal_map_name == default_normal_name)
		{
			normal_map_.texture = texture_system::get_default_diffuse_texture();
		}
		else
		{
			normal_map_.texture = texture_system::acquire(properties.normal_map_name);
		}
		shader_id_ = shader_system::get_shader_id(shader_name_);
	}

	void material::set_diffuse_colour(const float4 diffuse)
	{
		diffuse_colour_ = diffuse;
	}
}