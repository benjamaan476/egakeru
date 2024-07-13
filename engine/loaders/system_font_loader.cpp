#include "system_font_loader.h"
#include "systems/resource_system.h"

namespace egkr
{
	system_font_loader::unique_ptr system_font_loader::create(const loader_properties& properties)
	{
		return std::make_unique<system_font_loader>(properties);
	}

	system_font_loader::system_font_loader(const loader_properties& properties)
		:resource_loader(resource::type::system_font, properties)
	{}

	resource::shared_ptr system_font_loader::load(std::string_view name, void* /*params*/)
	{
		auto base_path = get_base_path();
		constexpr std::string_view format_string{ "%s/%s%s" };

		char buff[128]{};

		std::array<supported_system_font_file_type, 2> filetypes{ supported_system_font_file_type{".esf", system_font_file_type::esf, true},  supported_system_font_file_type{".fontcfg", system_font_file_type::font_config, false} };

		bool found{};
		supported_system_font_file_type filetype{};
		for (const auto& extension : filetypes)
		{
			sprintf_s(buff, format_string.data(), base_path.data(), name.data(), extension.extension.data());

			if (filesystem::does_path_exist(buff))
			{
				filetype = extension;
				found = true;
				break;
			}

		}

		if (!found)
		{
			LOG_ERROR("File not found: {}", buff);
			return nullptr;
		}


		font::system_font_resource_data resource_data{};
		auto file = filesystem::open(buff, file_mode::read, filetype.is_binary);

		switch (filetype.type)
		{
		case system_font_file_type::font_config:
		{
			sprintf_s(buff, format_string.data(), base_path.data(), name.data(), ".ebf");

			std::string ebf_file_name = buff;
			resource_data = import_fontcfg_file(file, ebf_file_name);
		} break;
		case system_font_file_type::esf:
		{
			resource_data = read_esf_file(file);
		} break;
		case system_font_file_type::not_found:
		default:
		{
			LOG_FATAL("Shouldn't have got here");
		}
		}

		resource::properties properties{ .type = resource::type::system_font, .name = name.data(), .full_path = buff, };
		properties.data = new font::system_font_resource_data();
		*((font::system_font_resource_data*)(properties.data)) = resource_data;
		return resource::create(properties);
	}

	bool system_font_loader::unload(const resource::shared_ptr& resource)
	{
		if (resource->data)
		{
			free(resource->data);
			resource->data = nullptr;
		}
		return true;
	}

	font::system_font_resource_data system_font_loader::import_fontcfg_file(file_handle& handle, std::string_view esf_filename)
	{
		font::system_font_resource_data data{};
		uint32_t line_number{};
		egkr::vector<uint8_t> line{ '#' };

		for (; !line.empty(); line = filesystem::read_line(handle, 511), ++line_number)
		{
			std::string line_string{ line.begin(), line.end() };
			trim(line_string);

			if (line[0] == '#' || line[0] == '\0')
			{
				continue;
			}

			auto split_index = line_string.find_first_of('=');
			if (split_index == std::string::npos)
			{
				LOG_WARN("Potential formatting issue found in file {}: '=' token not found on line number {}.", handle.filepath.data(), line_number);
				continue;
			}

			auto variable_name = line_string.substr(0, split_index);
			trim(variable_name);

			auto value = line_string.substr(split_index + 1);
			trim(value);

			if (variable_name == "version")
			{

			}
			else if (variable_name == "file")
			{
				auto font_file = value;
				auto font_resource = resource_system::load(font_file, resource::type::binary, nullptr);
				binary_resource_properties* font_data = (binary_resource_properties*)font_resource->data;
				data.binary_size = font_data->data.size();
				data.font_binary = malloc(data.binary_size);
				std::memcpy(data.font_binary, font_data->data.data(), data.binary_size);
				resource_system::unload(font_resource);
			}
			else if(variable_name == "face")
			{
				font::system_font_face new_face{};
				new_face.name = value;
				data.fonts.push_back(new_face);
			}
		}

		if (!data.font_binary || data.fonts.empty())
		{
			LOG_ERROR("Bad fontcfg file, {}. Required information missing", handle.filepath.data());
			return data;
		}

		return write_esf_file(esf_filename, data);
	}

	font::system_font_resource_data system_font_loader::read_esf_file(file_handle& /*handle*/)
	{
		return font::system_font_resource_data();
	}

	font::system_font_resource_data system_font_loader::write_esf_file(std::string_view /*path*/, const font::system_font_resource_data& data)
	{
		//TODO
		return data;
	}
}
