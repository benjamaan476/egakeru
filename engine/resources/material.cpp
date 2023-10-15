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
		diffuse_map_.use = texture_use::map_diffuse;
		diffuse_map_.texture = texture_system::get_default_texture();
		shader_id_ = shader_system::get_shader_id(shader_name_);
	}

	void material::set_diffuse_colour(const float4 diffuse)
	{
		diffuse_colour_ = diffuse;
	}
}