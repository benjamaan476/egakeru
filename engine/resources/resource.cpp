#include "resource.h"

namespace egkr
{
	resource::shared_ptr resource::create(const resource::properties& properties)
	{
		return std::make_shared<resource>(properties);
	}

	resource::resource(const resource::properties& properties)
		: data{ properties.data }, id_{0}, generation_{0}, name_{properties.name}, full_path_{properties.full_path}, resource_type_{properties.type}
	{
	}
}