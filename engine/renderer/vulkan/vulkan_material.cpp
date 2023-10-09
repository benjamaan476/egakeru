#include "vulkan_material.h"
#include "vulkan_types.h"

namespace egkr
{
	vulkan_material::shared_ptr vulkan_material::create(const vulkan_context* context, const material_properties& properties)
	{
		auto mat = std::make_shared<vulkan_material>(context, properties);
		switch (properties.type)
		{
		case material_type::world:

			if (!context->material_shader->acquire_resource(mat))
			{
				LOG_ERROR("Failed to acquire shader resource");
				return nullptr;
			}
			LOG_INFO("Acquired shader resource");
			break;
		case material_type::ui:
			if (!context->ui_shader->acquire_resource(mat))
			{
			}
			break;

		}
		return mat;
	}

	vulkan_material::vulkan_material(const vulkan_context* context, const material_properties& properties)
		: material{ properties }, context_{	context	}
	{

	}

	vulkan_material::~vulkan_material()
	{
		destroy();
	}

	void vulkan_material::destroy()
	{
	}
}