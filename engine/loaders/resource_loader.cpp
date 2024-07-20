#include "resource_loader.h"

namespace egkr
{
	resource_loader::resource_loader(resource::type type, const loader_properties& properties)
		: loader_type_{ type }, type_path_{properties.path}
	{
		if (properties.custom_type.has_value())
		{
			custom_type_name_ = *properties.custom_type;
		}
	}
}
