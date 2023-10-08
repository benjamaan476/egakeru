#include "geometry.h"

#include "renderer/vulkan/vulkan_geometry.h"
#include "renderer/vulkan/vulkan_types.h"

#include "systems/material_system.h"

namespace egkr
{
	geometry::shared_ptr geometry::create(const void* context, const geometry_properties& properties)
	{
		return std::make_shared<vulkan_geometry>((vulkan_context*)context, properties);
	}

	geometry::geometry(const geometry_properties& properties)
		: resource(0, 0), id_{ properties.id }, generation_{properties.generation}
	{
		material_ = material_system::acquire(properties.material_name);
	}

	geometry::~geometry()
	{

	}
}