#include "shader_loader.h"

#include "platform/filesystem.h"
#include <map>
#include <resources/shader.h>
#include "parser.h"

namespace egkr
{
    shader_loader::unique_ptr shader_loader::create(const loader_properties& properties) { return std::make_unique<shader_loader>(properties); }

    shader_loader::shader_loader(const loader_properties& properties): resource_loader{resource::type::shader, properties} { }

    resource::shared_ptr shader_loader::load(const std::string& name, void* /*params*/)
    {
	const auto base_path = get_base_path();
	const std::string filename = std::format("{}/{}{}", base_path, name, ".shadercfg");

	const auto properties = load_configuration_file(filename);

	resource::properties resource_properties{};
	resource_properties.type = get_loader_type();
	resource_properties.name = name;
	resource_properties.full_path = filename;

	resource_properties.data = new shader::properties();
	*(shader::properties*)resource_properties.data = properties;

	return resource::create(resource_properties);
    }

    bool shader_loader::unload(const resource::shared_ptr& resource)
    {
	delete (shader::properties*)resource->data;
	return true;
    }

    const std::map<std::string, std::function<void(shader::properties&, const std::string&)>> shader_configuration_members = {
        {"version", [](shader::properties& /* properties */, const std::string& /* value */) noexcept {}},
        {"name", [](shader::properties& properties, const std::string& value) noexcept { properties.name = value; }},
        {"depth_write",
            [](shader::properties& properties, const std::string& value) noexcept
            {
	        if (std::stoi(value))
	        {
	            properties.shader_flags |= shader::flags::depth_write;
	        }
            }},
        {"depth_test",
            [](shader::properties& properties, const std::string& value) noexcept
            {
	        if (std::stoi(value))
	        {
	            properties.shader_flags |= shader::flags::depth_test;
	        }
            }},
        {"stages",
            [](shader::properties& properties, const std::string& val) noexcept
            {
	        std::string stage{};
	        std::string value = val;
	        auto offset = value.find_first_of(',');
	        while (offset != std::string::npos)
	        {
	            stage = value.substr(0, offset);
	            shader::stages shader_stage;
	            if (stage == "frag" || stage == "fragment")
	            {
		        shader_stage = shader::stages::fragment;
	            }
	            else if (stage == "vert" || stage == "vertex")
	            {
		        shader_stage = shader::stages::vertex;
	            }
	            else if (stage == "geom" || stage == "geometry")
	            {
		        shader_stage = shader::stages::geometry;
	            }
	            else if (stage == "compute")
	            {
		        shader_stage = shader::stages::compute;
	            }
	            else
	            {
		        LOG_ERROR("Unrecognised shader type: {}", stage);
		        continue;
	            }
	            properties.shader_stages.push_back(shader_stage);
	            value = value.substr(offset + 1);
	            offset = value.find_first_of(',');
	        }

	        stage = value.substr(0, offset);
	        shader::stages shader_stage;
	        if (stage == "frag" || strcmp(stage.data(), "fragment") == 0)
	        {
	            shader_stage = shader::stages::fragment;
	        }
	        else if (stage == "vert" || stage == "vertex")
	        {
	            shader_stage = shader::stages::vertex;
	        }
	        else if (stage == "geom" || stage == "geometry")
	        {
	            shader_stage = shader::stages::geometry;
	        }
	        else if (stage == "compute")
	        {
	            shader_stage = shader::stages::compute;
	        }
	        else
	        {
	            LOG_ERROR("Unrecognised shader type: {}", stage);
	            return;
	        }
	        properties.shader_stages.push_back(shader_stage);
            }},
        {"stagefiles",
            [](shader::properties& properties, const std::string& val) noexcept
            {
	        std::string value = val;
	        auto offset = value.find_first_of(',');
	        while (offset != std::string::npos)
	        {
	            std::string filename = value.substr(0, offset);
	            properties.stage_filenames.push_back(filename);
	            value = value.substr(offset + 1);
	            offset = value.find_first_of(',');
	        }

	        std::string filename = value.substr(0, offset);
	        properties.stage_filenames.push_back(filename);
	        value = value.substr(offset + 1);
            }},
        {"cull_mode",
            [](shader::properties& properties, const std::string& value) noexcept
            {
	        if (value == "front")
	        {
	            properties.shader_cull_mode = shader::cull_mode::front;
	        }
	        else if (value == "back")
	        {
	            properties.shader_cull_mode = shader::cull_mode::back;
	        }
	        else if (value == "both")
	        {
	            properties.shader_cull_mode = shader::cull_mode::both;
	        }
            }},
        {"topology",
            [](shader::properties& properties, const std::string& val) noexcept
            {
	        std::string value = val;
	        shader::primitive_topology_type types{};
	        auto offset = value.find_first_of(',');
	        while (offset != std::string::npos)
	        {
	            std::string filename = value.substr(0, offset);
	            types |= shader::to_primitive_topology(filename);
	            value = value.substr(offset + 1);
	            offset = value.find_first_of(',');
	        }

	        std::string filename = value.substr(0, offset);
	        types |= shader::to_primitive_topology(filename);
	        value = value.substr(offset + 1);

	        properties.topology_types = types;
            }},
        {"attribute",
            [](shader::properties& properties, const std::string& value) noexcept
            {
	        auto offset = value.find_first_of(',');
	        if (offset == std::string::npos)
	        {
	            LOG_ERROR("Invalid attribute: {}", value);
	        }
	        else
	        {
	            auto type = value.substr(0, offset);
	            auto name = value.substr(offset + 1);

	            shader::attribute_configuration attribute{};

	            if (type == "f32")
	            {
		        attribute.type = shader::attribute_type::float32_1;
		        attribute.size = 4;
	            }
	            else if (type == "vec2")
	            {
		        attribute.type = shader::attribute_type::float32_2;
		        attribute.size = 8;
	            }
	            else if (type == "vec3")
	            {
		        attribute.type = shader::attribute_type::float32_3;
		        attribute.size = 12;
	            }
	            else if (type == "vec4")
	            {
		        attribute.type = shader::attribute_type::float32_4;
		        attribute.size = 16;
	            }
	            else if (type == "u8")
	            {
		        attribute.type = shader::attribute_type::uint8;
		        attribute.size = 1;
	            }
	            else if (type == "u16")
	            {
		        attribute.type = shader::attribute_type::uint16;
		        attribute.size = 2;
	            }
	            else if (type == "u32")
	            {
		        attribute.type = shader::attribute_type::uint32;
		        attribute.size = 4;
	            }
	            else if (type == "i8")
	            {
		        attribute.type = shader::attribute_type::int8;
		        attribute.size = 1;
	            }
	            else if (type == "i16")
	            {
		        attribute.type = shader::attribute_type::int16;
		        attribute.size = 2;
	            }
	            else if (type == "i32")
	            {
		        attribute.type = shader::attribute_type::uint32;
		        attribute.size = 4;
	            }
	            else
	            {
		        LOG_ERROR("Unknown attribute type found: {}", type);
	            }

	            attribute.name = name;

	            properties.attributes.push_back(attribute);
	        }
            }},
        {"uniform",
            [](shader::properties& properties, const std::string& val) noexcept
            {
	        std::string value = val;
	        auto offset = value.find_first_of(',');
	        if (offset == std::string::npos)
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

	            shader::uniform_configuration uniform{};

	            if (type == "f32")
	            {
		        uniform.type = shader::uniform_type::float32_1;
		        uniform.size = 4;
	            }
	            else if (type == "vec2")
	            {
		        uniform.type = shader::uniform_type::float32_2;
		        uniform.size = 8;
	            }
	            else if (type == "vec3")
	            {
		        uniform.type = shader::uniform_type::float32_3;
		        uniform.size = 12;
	            }
	            else if (type == "vec4")
	            {
		        uniform.type = shader::uniform_type::float32_4;
		        uniform.size = 16;
	            }
	            else if (type == "u8")
	            {
		        uniform.type = shader::uniform_type::uint8;
		        uniform.size = 1;
	            }
	            else if (type == "u16")
	            {
		        uniform.type = shader::uniform_type::uint16;
		        uniform.size = 2;
	            }
	            else if (type == "u32")
	            {
		        uniform.type = shader::uniform_type::uint32;
		        uniform.size = 4;
	            }
	            else if (type == "i8")
	            {
		        uniform.type = shader::uniform_type::int8;
		        uniform.size = 1;
	            }
	            else if (type == "i16")
	            {
		        uniform.type = shader::uniform_type::int16;
		        uniform.size = 2;
	            }
	            else if (type == "i32")
	            {
		        uniform.type = shader::uniform_type::uint32;
		        uniform.size = 4;
	            }
	            else if (type == "mat4")
	            {
		        uniform.type = shader::uniform_type::mat4x4;
		        uniform.size = 64;
	            }
	            else if (type == "samp")
	            {
		        uniform.type = shader::uniform_type::sampler;
		        uniform.size = 0;
	            }
	            else if (type == "struct32")
	            {
		        uniform.type = shader::uniform_type::custom;
		        uniform.size = 32;
	            }
	            else if (type == "struct128")
	            {
		        uniform.type = shader::uniform_type::custom;
		        uniform.size = 320;
	            }
	            else if (type == "struct480")
	            {
		        uniform.type = shader::uniform_type::custom;
		        uniform.size = 480;
	            }
	            else
	            {
		        LOG_ERROR("Unknown uniform type found: {}", type);
	            }

	            if (scope == "0")
	            {
		        uniform.shader_scope = shader::scope::global;
	            }
	            else if (scope == "1")
	            {
		        uniform.shader_scope = shader::scope::instance;
	            }
	            else if (scope == "2")
	            {
		        uniform.shader_scope = shader::scope::local;
	            }
	            else
	            {
		        LOG_ERROR("Unknown shader scope: {}", scope);
	            }

	            uniform.name = name;

	            properties.uniforms.push_back(uniform);
	        }
            }},
    };

    shader::properties shader_loader::load_configuration_file(std::string_view path)
    {
	shader::properties properties{};
	properties.shader_cull_mode = shader::cull_mode::back;
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
	    std::string line_string{line.begin(), line.end()};
	    trim(line_string);

	    if (const auto& parse_result = parser::parse_line(line_string))
	    {
		const auto& [variable_name, value] = parse_result.value();
		if (!shader_configuration_members.contains(variable_name))
		{
		    LOG_WARN("Unrecognised terrain configuration argument: {}", variable_name);
		    continue;
		}
		shader_configuration_members.at(variable_name)(properties, value);
	    }
	    else
	    {
		continue;
	    }
	}
	return properties;
    }
}
