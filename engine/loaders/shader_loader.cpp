#include "shader_loader.h"

#include "platform/filesystem.h"

namespace egkr
{
	shader_loader::unique_ptr shader_loader::create(const loader_properties& properties)
	{
		return std::make_unique<shader_loader>(properties);
	}

	shader_loader::shader_loader(const loader_properties& properties)
		: resource_loader{ resource_type::shader, properties }
	{

	}

	resource::shared_ptr shader_loader::load(std::string_view name)
	{
		const auto base_path = get_base_path();
		constexpr std::string_view format_string{ "%s/%s" };

		char filename[128];
		sprintf_s(filename, format_string.data(), base_path.data(), name.data());

		const auto properties = load_configuration_file(filename);

		resource_properties resource_properties{};
		resource_properties.type = get_loader_type();
		resource_properties.name = name;
		resource_properties.full_path = filename;

		resource_properties.data = new shader_properties();
		*(shader_properties*)resource_properties.data = properties;

		return std::make_shared<resource>(resource_properties);
	}

	bool shader_loader::unload(const resource::shared_ptr& resource)
	{
		delete (shader_properties*)resource->data;
		return true;
	}

	shader_properties shader_loader::load_configuration_file(std::string_view path)
	{
		shader_properties properties{};

		auto handle = filesystem::open(path, file_mode::read, false);
		if (!handle.is_valid)
		{
			LOG_ERROR("Failed to open binary file: {}", path.data());
			return {};
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
			else if (variable_name == "renderpass_name")
			{
				properties.renderpass_name = value;
			}
			else if (variable_name == "stages")
			{
				std::string stage{};
				while (auto offset = value.find_first_of(',') != std::string::npos)
				{
					stage = value.substr(0, offset);
					shader_stages shader_stage;
					if (stage == "frag" || stage == "fragment")
					{
						shader_stage = shader_stages::fragment;
					}
					else if (stage == "vert" || stage == "vertex")
					{
						shader_stage = shader_stages::vertex;
					}
					else if (stage == "geom" || stage == "geometry")
					{
						shader_stage = shader_stages::geometry;
					}
					else if (stage == "compute")
					{
						shader_stage = shader_stages::compute;
					}
					else
					{
						LOG_ERROR("Unrecognised shader type: {}", stage);
						continue;
					}
					properties.stages.push_back(shader_stage);
					value = value.substr(offset + 1);
				}
			}
			else if (variable_name == "stage_files")
			{
				while (auto offset = value.find_first_of(',') != std::string::npos)
				{
					std::string filename = value.substr(0, offset);
					properties.stage_filenames.push_back(filename);
					value = value.substr(offset + 1);
				}
			}
			else if (variable_name == "use_instance")
			{
				if (value == "true")
				{
					properties.use_instance = true;
				}
			}
			else if (variable_name == "use_local")
			{
				if (value == "true")
				{
					properties.use_local = true;
				}
			}
			else if (variable_name == "attribute")
			{
				if (auto offset = value.find_first_of(',') == std::string::npos)
				{
					LOG_ERROR("Invalid attribute: {}", value);
				}
				else
				{
					auto type = value.substr(0, offset);
					auto name = value.substr(offset + 1);

					attribute_configuration attribute{};

					if (type == "f32")
					{
						attribute.type = shader_attribute_type::float32_1;
						attribute.size = 4;
					}
					else if (type == "vec2")
					{
						attribute.type = shader_attribute_type::float32_2;
						attribute.size = 8;
					}
					else if (type == "vec3")
					{
						attribute.type = shader_attribute_type::float32_3;
						attribute.size = 12;
					}
					else if (type == "vec4")
					{
						attribute.type = shader_attribute_type::float32_4;
						attribute.size = 16;
					}
					else if (type == "u8")
					{
						attribute.type = shader_attribute_type::uint8;
						attribute.size = 1;
					}
					else if (type == "u16")
					{
						attribute.type = shader_attribute_type::uint16;
						attribute.size = 2;
					}
					else if (type == "u32")
					{
						attribute.type = shader_attribute_type::uint32;
						attribute.size = 4;
					}
					else if (type == "i8")
					{
						attribute.type = shader_attribute_type::int8;
						attribute.size = 1;
					}
					else if (type == "i16")
					{
						attribute.type = shader_attribute_type::int16;
						attribute.size = 2;
					}
					else if (type == "i32")
					{
						attribute.type = shader_attribute_type::uint32;
						attribute.size = 4;
					}
					else
					{
						LOG_ERROR("Unknown attribute type found: {}", type);
					}

					attribute.name = name;

					properties.attributes.push_back(attribute);
				}
			}
			else if (variable_name == "uniform")
			{
				if (auto offset = value.find_first_of(',') == std::string::npos)
				{
					LOG_ERROR("Invalid uniform: {}", value);
				}
				else
				{
					auto type = value.substr(0, offset);
					value = value.substr(offset + 1);
					auto offset2 = value.find_first_of(',');
					auto scope = value.substr(0, offset2);
					auto name = value.substr(offset2 + 1);

					uniform_configuration uniform{};

					if (type == "f32")
					{
						uniform.type = shader_uniform_type::float32_1;
						uniform.size = 4;
					}
					else if (type == "vec2")
					{
						uniform.type = shader_uniform_type::float32_2;
						uniform.size = 8;
					}
					else if (type == "vec3")
					{
						uniform.type = shader_uniform_type::float32_3;
						uniform.size = 12;
					}
					else if (type == "vec4")
					{
						uniform.type = shader_uniform_type::float32_4;
						uniform.size = 16;
					}
					else if (type == "u8")
					{
						uniform.type = shader_uniform_type::uint8;
						uniform.size = 1;
					}
					else if (type == "u16")
					{
						uniform.type = shader_uniform_type::uint16;
						uniform.size = 2;
					}
					else if (type == "u32")
					{
						uniform.type = shader_uniform_type::uint32;
						uniform.size = 4;
					}
					else if (type == "i8")
					{
						uniform.type = shader_uniform_type::int8;
						uniform.size = 1;
					}
					else if (type == "i16")
					{
						uniform.type = shader_uniform_type::int16;
						uniform.size = 2;
					}
					else if (type == "i32")
					{
						uniform.type = shader_uniform_type::uint32;
						uniform.size = 4;
					}
					else if (type == "mat4")
					{
						uniform.type = shader_uniform_type::mat4x4;
						uniform.size = 64;
					}
					else if (type == "sampler")
					{
						uniform.type = shader_uniform_type::sampler;
						uniform.size = 0;
					}
					else
					{
						LOG_ERROR("Unknown attribute type found: {}", type);
					}

					if (scope == "0")
					{
						uniform.scope = shader_scope::global;
					}
					else if (scope == "1")
					{
						uniform.scope = shader_scope::instance;
					}
					else if (scope == "2")
					{
						uniform.scope = shader_scope::local;
					}
					else
					{
						LOG_ERROR("Unknown shader scope: {}", scope);
					}

					uniform.name = name;

					properties.uniforms.push_back(uniform);
				}
			}
		}
		return properties;
	}
}