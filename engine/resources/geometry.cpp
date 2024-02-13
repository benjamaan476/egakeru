#include "geometry.h"

#include "systems/material_system.h"
#include "renderer/renderer_types.h"

namespace egkr::geometry
{
	geometry::shared_ptr geometry::create(const renderer_backend* backend, const properties& properties)
	{
		return backend->create_geometry(properties);
	}

	geometry::geometry(const renderer_backend* backend, const properties& properties)
		: resource(properties.id, properties.generation, properties.name), properties_{properties}, backend_{ backend }
	{
		if (properties.material_name == default_material_name_)
		{
			material_ = material_system::get_default_material();
		}
		else
		{
			material_ = material_system::acquire(properties.material_name);
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