#include "material.h"

#include "renderer/renderer_frontend.h"

#include "systems/texture_system.h"

namespace egkr
{
	material::shared_ptr material::create(const renderer_frontend* renderer, const material_properties& properties)
	{
		auto mat = std::make_shared<material>(properties);
		if (!renderer->populate_material(mat.get()))
		{
			LOG_ERROR("Failed to populate material");
			return nullptr;
		}

		return mat;
	}

	material::material(const material_properties& properties)
		: resource(0, 0)
	{
		type_ = properties.type;
		diffuse_map_.use = texture_use::map_diffuse;
		diffuse_map_.texture = texture_system::get_default_texture();
	}

	void material::set_diffuse_colour(const float4 diffuse)
	{
		diffuse_colour_ = diffuse;
	}
}