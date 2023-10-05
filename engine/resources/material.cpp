#include "material.h"

#include "systems/texture_system.h"
#include "resources/material.h"
#include "renderer/vulkan/vulkan_material.h"

namespace egkr
{
	material::shared_ptr material::create(const void* renderer_context)
	{
		return vulkan_material::create((vulkan_context*)renderer_context);
	}

	material::material()
	{
		diffuse_map_.use = texture_use::map_diffuse;
		diffuse_map_.texture = texture_system::get_default_texture();
	}

	void material::set_diffuse_colour(const float4 diffuse)
	{
		diffuse_colour_ = diffuse;
	}
}