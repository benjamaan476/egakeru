#include "material_loader.h"
#include "platform/filesystem.h"

namespace egkr
{
	material_loader::unique_ptr material_loader::create(const loader_properties& properties)
	{
		return std::make_unique<material_loader>(properties);
	}

	material_loader::material_loader(const loader_properties& properties)
		:resource_loader{ resource_type::material, properties }
	{
	}

	resource::shared_ptr material_loader::load(std::string_view name)
	{
		const auto base_path = get_base_path();
		constexpr std::string_view format_string{ "%s/%s%s" };

		char material_filename[128];
		sprintf_s(material_filename, format_string.data(), base_path.data(), name.data(), ".emt");

		const auto properties = load_configuration_file(material_filename);

		resource_properties resource_properties{};
		resource_properties.type = get_loader_type();
		resource_properties.name = name;
		resource_properties.full_path = material_filename;
		resource_properties.data = new(material_properties);

		*(material_properties*)(resource_properties.data) = properties;

		return std::make_shared<resource>(resource_properties);
	}

	bool material_loader::unload(const resource::shared_ptr& resource)
	{
		auto* data = (material_properties*)resource->data;
		delete data;
		return false;
	}

	material_properties material_loader::load_configuration_file(std::string_view path)
	{
		material_properties properties{};

		auto handle = filesystem::open(path, file_mode::read, false);
		if (!handle.is_valid)
		{
			LOG_ERROR("Failed to open material file: {}", path.data());
			properties.diffuse_colour = float4{ 1.F };
			properties.diffuse_map_name = "default";
			properties.diffuse_map_name = "default";
			return properties;
		}

		auto line = filesystem::read_line(handle, 511);
		uint32_t line_number{};
		for (; !line.empty(); line = filesystem::read_line(handle, 511), ++line_number)
		{
			std::string line_string{ line.begin(), line.end() };
			trim(line_string);

			if (line.empty() || line[0] == '#' || line[0] == '\0')
			{
				//line = filesystem::read_line(handle, 511);
				continue;
			}

			auto split_index = line_string.find_first_of('=');
			if (split_index == std::string::npos)
			{
				LOG_WARN("Potential formatting issue found in file {}: '=' token not found on line number {}.", path.data(), line_number);
				continue;
			}

			auto variable_name = line_string.substr(0, split_index);
			trim(variable_name);

			auto value = line_string.substr(split_index + 1);
			trim(value);

			if (variable_name == "version")
			{
				//TODO version
			}
			else if (variable_name == "name")
			{
				properties.name = value;
			}
			else if (variable_name == "diffuse_map_name")
			{
				properties.diffuse_map_name = value;
			}
			else if (variable_name == "specular_map_name")
			{
				properties.specular_map_name = value;
			}
			else if (variable_name == "diffuse_colour")
			{
				std::stringstream ss{ value };
				float x, y, z, w;
				ss >> x >> y >> z >> w;

				properties.diffuse_colour = { x, y, z, w };
			}
			else if (variable_name == "shader")
			{
				properties.shader_name = value;
			}
			else if (variable_name == "shininess")
			{
				std::stringstream ss{value};
				float shininess{};
				ss >> shininess;
				properties.shininess = shininess;
			}
		}
		return properties;
	}
}