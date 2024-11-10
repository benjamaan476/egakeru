#include "resource.h"

namespace egkr
{
	resource::shared_ptr resource::create(const resource::properties& resource_properties)
	{
		return std::make_shared<resource>(resource_properties);
	}

	resource::resource(const resource::properties& resource_properties)
		: data{ resource_properties.data }, id_{0}, generation_{0}, name_{resource_properties.name}, full_path_{resource_properties.full_path}, resource_type_{resource_properties.type}
	{
	}
}
