#include "binary_loader.h"

#include "platform/filesystem.h"

namespace egkr
{
	binary_loader::unique_ptr binary_loader::create(const loader_properties& properties)
	{
		return std::make_unique<binary_loader>(properties);
	}

	binary_loader::binary_loader(const loader_properties& properties)
		:resource_loader{ resource_type::binary, properties }
	{
	}

	resource::shared_ptr binary_loader::load(std::string_view name, void* /*params*/)
	{
		//bianry file needs full filename
		const auto base_path = get_base_path();
		constexpr std::string_view format_string{ "%s/%s" };

		char filename[128];
		sprintf_s(filename, format_string.data(), base_path.data(), name.data());


		auto handle = filesystem::open(filename, file_mode::read, true);
		if (!handle.is_valid)
		{
			LOG_ERROR("Failed to open binary file: {}", filename);
			return {};
		}

		auto line = filesystem::read_all(handle);

		resource_properties properties{};
		properties.type = get_loader_type();
		properties.name = name;
		properties.full_path = name;
		properties.data = new(binary_resource_properties);

		binary_resource_properties binary_properties{ line };
		*(binary_resource_properties*)properties.data = binary_properties;

		return std::make_shared<resource>(properties);
	}

	bool binary_loader::unload(const resource::shared_ptr& resource)
	{
		auto* data = (binary_resource_properties*)resource->data;
		data->data.clear();
		delete data;
		return true;
	}
}