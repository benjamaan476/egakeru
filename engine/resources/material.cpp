#include "material.h"

#include "systems/texture_system.h"
#include "systems/shader_system.h"
#include "resources/material.h"
#include "renderer/vulkan/vulkan_material.h"

namespace egkr
{
	material::shared_ptr material::create(const void* renderer_context, const material_properties& properties)
	{
		return vulkan_material::create((vulkan_context*)renderer_context, properties);
	}

	material::material(const material_properties& properties)
		: resource(0, 0), name_{ properties.name }, diffuse_colour_{ properties.diffuse_colour }, shader_name_{ properties.shader_name }
	{
		diffuse_map_.use = texture_use::map_diffuse;
		diffuse_map_.texture = texture_system::get_default_texture();
		shader_id_ = shader_system::get_shader_id(shader_name_);
	}

	void material::set_diffuse_colour(const float4 diffuse)
	{
		diffuse_colour_ = diffuse;
	}
}