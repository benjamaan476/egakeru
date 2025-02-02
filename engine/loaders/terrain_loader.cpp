#include "terrain_loader.h"

#include "log/log.h"
#include "platform/filesystem.h"
#include "resources/resource.h"
#include "resources/terrain.h"
#include "systems/resource_system.h"
#include <map>

namespace egkr::loader
{
    terrain::unique_ptr terrain::create(const loader_properties& properties) { return std::make_unique<terrain>(properties); }

    terrain::terrain(const loader_properties& properties): resource_loader{resource::type::terrain, properties} { }

    resource::shared_ptr terrain::load(const std::string& name, void* /* params */)
    {
	const auto base_path = get_base_path();
	const std::string filename = std::format("{}/{}{}", base_path, name, ".eterrain");

	const auto properties = load_configuration_file(filename);

	resource::properties resource_properties{.type = get_loader_type(), .name = name, .full_path = filename, .data = new egkr::terrain::properties()};

	resource_properties.data = new egkr::terrain::configuration();
	*(egkr::terrain::configuration*)resource_properties.data = properties;

	return resource::create(resource_properties);
    }

    bool terrain::unload(const resource::shared_ptr& resource)
    {
	delete (egkr::terrain::configuration*)resource->data;
	resource->data = nullptr;
	return true;
    }

    static const std::map<std::string, std::function<void(egkr::terrain::configuration&, const std::string&)>> terrain_configuration_members = {
        {"version", [](egkr::terrain::configuration& /* config */, const std::string& /* value */) noexcept {}},
        {"heightmap", [](egkr::terrain::configuration& config, const std::string& value) noexcept { config.heightmap = value; }},
        {"tiles_x", [](egkr::terrain::configuration& config, const std::string& value) noexcept { config.tiles_x = (uint32_t)std::stoi(value); }},
        {"tiles_y", [](egkr::terrain::configuration& config, const std::string& value) noexcept { config.tiles_y = (uint32_t)std::stoi(value); }},
        {"scale_x", [](egkr::terrain::configuration& config, const std::string& value) noexcept { config.scale_x = std::stof(value); }},
        {"scale_y", [](egkr::terrain::configuration& config, const std::string& value) noexcept { config.scale_y = std::stof(value); }},
        {"scale_z", [](egkr::terrain::configuration& config, const std::string& value) noexcept { config.scale_z = std::stof(value); }},
    };

    egkr::terrain::configuration terrain::load_configuration_file(const std::string& path)
    {
	egkr::terrain::configuration properties{};

	auto handle = filesystem::open(path, file_mode::read, false);
	if (!handle.is_valid)
	{
	    LOG_ERROR("Failed to open terrain file {}", path);
	    return {};
	}

	auto line = filesystem::read_line(handle, 511);
	uint32_t line_number{};
	for (; !line.empty(); line = filesystem::read_line(handle, 511), ++line_number)
	{
	    std::string line_string{line.begin(), line.end()};
	    trim(line_string);

	    if (line[0] == '#' || line[0] == '\0')
	    {
		continue;
	    }

	    auto split_index = line_string.find_first_of('=');
	    if (split_index == std::string::npos)
	    {
		LOG_WARN("Potential formatting issue found in file {}, '=' token not found on line number {}.", path, line_number);
		continue;
	    }

	    auto variable_name = line_string.substr(0, split_index);
	    trim(variable_name);

	    auto value = line_string.substr(split_index + 1);
	    trim(value);

	    if (!terrain_configuration_members.contains(variable_name))
	    {
		LOG_WARN("Unrecognised terrain configuration arguement: {}", variable_name);
		continue;
	    }
	    terrain_configuration_members.at(variable_name)(properties, value);
	}

	if (properties.heightmap != "")
	{
	    image_resource_parameters params{.flip_y = false};
	    auto heightmap_image = resource_system::load(properties.heightmap, resource::type::image, &params);
	    if (!heightmap_image || !heightmap_image->data)
	    {
		LOG_ERROR("Could not load heightmap image: {}", properties.heightmap);
	    }
	    else
	    {
		auto* image_properties = (texture::properties*)heightmap_image->data;
		const auto* pixels = (uint8_t*)image_properties->data;
		properties.tiles_x = image_properties->width;
		properties.tiles_y = image_properties->height;

		const uint32_t count = properties.tiles_x * properties.tiles_y;
		properties.height_data.reserve(count);
		for (uint32_t i{}; i < 4 * count; i += 4)
		{
		    const auto r = pixels[i + 0];
		    const auto g = pixels[i + 1];
		    const auto b = pixels[i + 2];

		    const float sum = (r + g + b) / (3.f * 255);
		    properties.height_data.push_back(sum);
		}

		egkr::resource_system::unload(heightmap_image);
	    }
	}

	return properties;
    }
}
