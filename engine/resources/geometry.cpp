#include "geometry.h"

#include "systems/material_system.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
	geometry::shared_ptr geometry::create(const renderer_frontend* context, const geometry_properties& properties)
	{
		auto geom = std::make_shared<geometry>(properties);
		context->populate_geometry(geom.get(), properties);

		return geom;
	}

	geometry::geometry(const geometry_properties& properties)
		: resource(properties.id, properties.generation)
	{
		material_ = material_system::acquire(properties.material_name);
	}

	geometry::~geometry() = default;

	void geometry::destroy(const renderer_frontend* renderer)
	{
		renderer->free_geometry(this);
	}
}