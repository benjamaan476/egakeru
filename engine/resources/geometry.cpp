#include "geometry.h"

#include "systems/material_system.h"
#include "engine/engine.h"

namespace egkr
{
	geometry::shared_ptr geometry::create(const geometry::properties& properties)
	{
		return engine::get()->get_renderer()->create_geometry(properties);
	}

	geometry::geometry(const geometry::properties& geometry_properties)
		: resource(geometry_properties.id, geometry_properties.generation, geometry_properties.name), properties_{geometry_properties}
	{
		if (geometry_properties.material_name == default_material_name_)
		{
			material_ = material_system::get_default_material();
		}
		else
		{
			if (geometry_properties.material_name != "")
			{
				material_ = material_system::acquire(geometry_properties.material_name);
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
