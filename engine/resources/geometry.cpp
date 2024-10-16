#include "geometry.h"

#include "systems/material_system.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
	geometry::shared_ptr geometry::create(const geometry::properties& properties)
	{
		return renderer->create_geometry(properties);
	}

	geometry::geometry(const geometry::properties& properties)
		: resource(properties.id, properties.generation, properties.name), properties_{properties}
	{
		if (properties.material_name == default_material_name_)
		{
			material_ = material_system::get_default_material();
		}
		else
		{
			if (properties.material_name != "")
			{
				material_ = material_system::acquire(properties.material_name);
			}
		}
	}

	void geometry::destroy()
	{
		free();
	}

	void geometry::set_material(const material::shared_ptr& material)
	{
		material_ = material;
	}
}
