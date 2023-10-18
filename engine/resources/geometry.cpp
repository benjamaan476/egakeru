#include "geometry.h"

#include "systems/material_system.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
	geometry::shared_ptr geometry::create(const renderer_frontend* context, const geometry_properties& properties)
	{
		auto geom = std::make_shared<geometry>(properties);

		if (properties.vertex_count)
		{
			context->populate_geometry(geom.get(), properties);
		}
		else
		{
			LOG_WARN("Geometry has no vertex data. Not populating");
		}
		return geom;
	}

	geometry::geometry(const geometry_properties& properties)
		: resource(properties.id, properties.generation, properties.name)
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

	geometry::~geometry() = default;

	void geometry::destroy(const renderer_frontend* renderer)
	{
		renderer->free_geometry(this);
	}

	void geometry::set_material(const material::shared_ptr& material)
	{
		material_ = material;
	}
}