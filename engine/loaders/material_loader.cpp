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

    const std::map<std::string, std::function<void(material::properties&, const std::string&)>> material_configuration_members = {
        {"version", [](material::properties& /* config */, const std::string& /* value */) noexcept {}},
        {"name", [](material::properties& config, const std::string& value) noexcept { config.name = value; }},
        {"type", [](material::properties& config, const std::string& value) noexcept { config.material_type = (value == "phong") ? material::type::phong : material::type::pbr; }},
        {"diffuse_map_name", [](material::properties& config, const std::string& value) noexcept { config.diffuse_map_name = value; }},
        {"specular_map_name", [](material::properties& config, const std::string& value) noexcept { config.specular_map_name = value; }},
        {"normal_map_name", [](material::properties& config, const std::string& value) noexcept { config.normal_map_name = value; }},
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
    };

    material::properties material_loader::load_configuration_file(std::string_view path)
    {
	material::properties properties{};

	auto handle = filesystem::open(path, file_mode::read, false);
	if (!handle.is_valid)
	{
	    properties.diffuse_colour = float4{1.F};
	    properties.diffuse_map_name = default_diffuse_name;
	    properties.specular_map_name = default_specular_name;
	    properties.normal_map_name = default_normal_name;
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
	return properties;
    }
}
