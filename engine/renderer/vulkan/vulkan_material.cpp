#include "vulkan_material.h"
#include "vulkan_types.h"

namespace egkr
{
	vulkan_material::shared_ptr vulkan_material::create(const vulkan_context* context)
	{
		auto mat = std::make_shared<vulkan_material>(context);
		if (!context->material_shader->acquire_resource(mat))
		{
			LOG_ERROR("Failed to acquire shader resource");
			return nullptr;
		}
		LOG_INFO("Acquired shader resource");
		return mat;
	}

	vulkan_material::vulkan_material(const vulkan_context* context)
		: context_{context}
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