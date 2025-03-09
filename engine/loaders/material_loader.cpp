#include "material_loader.h"
#include "systems/texture_system.h"
#include "platform/filesystem.h"
#include "parser.h"

namespace egkr
{
    material_loader::unique_ptr material_loader::create(const loader_properties& properties) { return std::make_unique<material_loader>(properties); }

    material_loader::material_loader(const loader_properties& properties): resource_loader{resource::type::material, properties} { }

    resource::shared_ptr material_loader::load(const std::string& name, void* /*params*/)
    {
	const auto base_path = get_base_path();
	const std::string material_filename = std::format("{}/{}{}", base_path, name, ".emt");

	const auto properties = load_configuration_file(material_filename);

	resource::properties resource_properties{};
	resource_properties.type = get_loader_type();
	resource_properties.name = name;
	resource_properties.full_path = material_filename;
	resource_properties.data = new (material::properties);

	*(material::properties*)(resource_properties.data) = properties;

	return resource::create(resource_properties);
    }

    bool material_loader::unload(const resource::shared_ptr& resource)
    {
	auto* data = (material::properties*)resource->data;
	delete data;
	return false;
    }

    static enum class parse_mode : uint8_t { root, map, property } parse_mode;

    static struct map
    {
	texture_map::properties map_properties;
	std::string map_name;
	std::string texture_name;
    } current_map;

    static struct property
    {
	std::string name;
	std::string value;
	std::string type;
    } current_property;

    const std::map<std::string, std::function<void(material::properties&, const std::string&)>> material_configuration_members = {
        {"version", [](material::properties& /* config */, const std::string& /* value */) noexcept {}},
        {"name",
            [](material::properties& config, const std::string& value) noexcept
            {
	        if (parse_mode == parse_mode::root)
	        {
	            config.name = value;
	        }
	        else if (parse_mode == parse_mode::map)
	        {
	            current_map.map_name = value;
	        }
            }},
        {"type", [](material::properties& config, const std::string& value) noexcept { config.material_type = (value == "phong") ? material::type::phong : material::type::pbr; }},
        {"diffuse_map_name", [](material::properties& config, const std::string& value) noexcept { config.texture_maps.emplace("diffuse", std::make_pair(value, texture_map::properties{})); }},
        {"specular_map_name", [](material::properties& config, const std::string& value) noexcept { config.texture_maps.emplace("specular", std::make_pair(value, texture_map::properties{})); }},
        {"normal_map_name", [](material::properties& config, const std::string& value) noexcept { config.texture_maps.emplace("normal", std::make_pair(value, texture_map::properties{})); }},
        {"diffuse_colour",
            [](material::properties& config, const std::string& value) noexcept
            {
	        std::stringstream ss{value};
	        float x, y, z, w;
	        ss >> x >> y >> z >> w;

	        config.diffuse_colour = {x, y, z, w};
            }},
        {"shader", [](material::properties& config, const std::string& value) noexcept { config.shader_name = value; }},
        {"shininess", [](material::properties& config, const std::string& value) noexcept { config.shininess = std::stof(value); }},
        {"[map]",
            [](material::properties& /* config */, const std::string& /* value */) noexcept
            {
	        current_map = {};
	        if (parse_mode == parse_mode::map)
	        {
	            LOG_ERROR("Cannot have nested [map], potentially mismatched tags.");
	        }
	        parse_mode = parse_mode::map;
            }},
        {"[/map]",
            [](material::properties& config, const std::string& /* value */) noexcept
            {
	        parse_mode = parse_mode::root;

	        if (current_map.map_name.empty())
	        {
	            LOG_ERROR("Map does not have a valid name, cannot assign");
	            return;
	        }

	        config.texture_maps.emplace(current_map.map_name, std::make_pair(current_map.texture_name, current_map.map_properties));
            }},
        {"texture_name", [](material::properties& /* config */, const std::string& value) noexcept { current_map.texture_name = value; }},
        {"filter_min",
            [](material::properties& /* config */, const std::string& value) noexcept { current_map.map_properties.minify = (value == "linear") ? texture_map::filter::linear : texture_map::filter::nearest; }},
        {"filter_mag",
            [](material::properties& /* config */, const std::string& value) noexcept { current_map.map_properties.magnify = (value == "linear") ? texture_map::filter::linear : texture_map::filter::nearest; }},
        {"repeat_u",
            [](material::properties& /* config */, const std::string& value) noexcept
            {
	        if (value == "repeat")
	        {
	            current_map.map_properties.repeat_u = texture_map::repeat::repeat;
	        }
	        else if (value == "repeat_mirrored")
	        {
	            current_map.map_properties.repeat_u = texture_map::repeat::mirrored_repeat;
	        }
	        else if (value == "clamp_to_edge")
	        {
	            current_map.map_properties.repeat_u = texture_map::repeat::clamp_to_edge;
	        }
	        else if (value == "clamp_to_border")
	        {
	            current_map.map_properties.repeat_u = texture_map::repeat::clamp_to_border;
	        }
            }},
        {"repeat_v",
            [](material::properties& /* config */, const std::string& value) noexcept
            {
	        if (value == "repeat")
	        {
	            current_map.map_properties.repeat_v = texture_map::repeat::repeat;
	        }
	        else if (value == "repeat_mirrored")
	        {
	            current_map.map_properties.repeat_v = texture_map::repeat::mirrored_repeat;
	        }
	        else if (value == "clamp_to_edge")
	        {
	            current_map.map_properties.repeat_v = texture_map::repeat::clamp_to_edge;
	        }
	        else if (value == "clamp_to_border")
	        {
	            current_map.map_properties.repeat_v = texture_map::repeat::clamp_to_border;
	        }
            }},
        {"repeat_w",
            [](material::properties& /* config */, const std::string& value) noexcept
            {
	        if (value == "repeat")
	        {
	            current_map.map_properties.repeat_w = texture_map::repeat::repeat;
	        }
	        else if (value == "repeat_mirrored")
	        {
	            current_map.map_properties.repeat_w = texture_map::repeat::mirrored_repeat;
	        }
	        else if (value == "clamp_to_edge")
	        {
	            current_map.map_properties.repeat_w = texture_map::repeat::clamp_to_edge;
	        }
	        else if (value == "clamp_to_border")
	        {
	            current_map.map_properties.repeat_w = texture_map::repeat::clamp_to_border;
	        }
            }},
    };

    static std::map<std::string, material::properties> loaded_materials;

    material::properties material_loader::load_configuration_file(std::string_view path)
    {
	if (loaded_materials.contains(path.data()))
	{
	    return loaded_materials[path.data()];
	}

	material::properties properties{};

	auto handle = filesystem::open(path, file_mode::read, false);
	if (!handle.is_valid)
	{
	    properties.diffuse_colour = float4{1.F};
	    return properties;
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
		if (!material_configuration_members.contains(variable_name))
		{
		    LOG_WARN("Unrecognised material configuration argument: {}, with value {} on line", variable_name, value, line_number);
		    continue;
		}
		material_configuration_members.at(variable_name)(properties, value);
	    }
	    else
	    {
		continue;
	    }
	}
	loaded_materials.emplace(path, properties);
	return properties;
    }
}
