#include "resource.h"

namespace egkr
{
	resource::shared_ptr create(const resource_properties& properties)
	{
		return std::make_shared<resource>(properties);
	}

	resource::resource(const resource_properties& properties)
		: id_{0}, generation_{0}, name_{properties.name}, full_path_{properties.full_path}, resource_type_{properties.type}, data_{properties.data}
	{

	}
}