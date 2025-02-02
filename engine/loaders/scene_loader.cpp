#include "pch.h"
#include "scene_loader.h"

#include "platform/filesystem.h"
#include "scenes/simple_scene.h"
#include <map>

namespace egkr
{
    [[nodiscard]] scene::configuration load_configuration_file(file_handle& handle);

    enum class parse_mode : uint8_t
    {
	root,
	scene,
	directional_light,
	point_light,
	skybox,
	mesh,
	terrain
    };

    scene_loader::unique_ptr scene_loader::create(const loader_properties& properties) { return std::make_unique<scene_loader>(properties); }

    scene_loader::scene_loader(const loader_properties& properties): resource_loader(resource::type::scene, properties) { }

    resource::shared_ptr scene_loader::load(const std::string& name, void* /*params*/)
    {
	const auto base_path = get_base_path();
	const std::string filename = std::format("{}/{}", base_path, name);

	auto handle = filesystem::open(filename, file_mode::read, false);
	if (!handle.is_valid)
	{
	    LOG_ERROR("Failed to load scene {}", name.data());
	    return nullptr;
	}

	const auto scene_configuration = load_configuration_file(handle);

	resource::properties properties{
	    .type = resource::type::scene,
	    .name = name.data(),
	    .full_path = filename,
	};

	properties.data = new scene::configuration();
	*(scene::configuration*)properties.data = scene_configuration;

	return resource::create(properties);
    }

    bool scene_loader::unload(const resource::shared_ptr& resource)
    {
	auto* data = (scene::configuration*)resource->data;
	delete data;
	return true;
    }

    static parse_mode mode{parse_mode::root};
    static scene::mesh_scene_configuration current_mesh{};
    static scene::terrain_scene_configuration current_terrain{};
    static scene::point_light_scene_configuration current_point_light{};

    const std::map<std::string, std::function<void(scene::configuration&, const std::string&)>> scene_configuration_members = {{"!version",
                                                                                                                                   [](scene::configuration& config, const std::string& value)
                                                                                                                                   {
	                                                                                                                               if (mode != parse_mode::root)
	                                                                                                                               {
	                                                                                                                                   LOG_ERROR("Attempting to parse version in non-root parsing mode");
	                                                                                                                                   return;
	                                                                                                                               }
	                                                                                                                               config.version = std::stoi(value);

	                                                                                                                               if (config.version != 1)
	                                                                                                                               {
	                                                                                                                                   LOG_WARN("Scene version exceeds known version number");
	                                                                                                                               }
                                                                                                                                   }},
        {"[Scene]",
            [](scene::configuration& /* config */, const std::string& /* value */)
            {
	        if (mode != parse_mode::root)
	        {
	            LOG_ERROR("Cannot enter scene parsing from non-root parse mode");
	            return;
	        }
	        mode = parse_mode::scene;
            }},
        {"[/Scene]",
            [](scene::configuration& /* config */, const std::string& /* value */)
            {
	        if (mode != parse_mode::scene)
	        {
	            LOG_ERROR("Cannot leave scene parsing from non-scene parse mode. Potential mismatched tag?");
	            return;
	        }
	        mode = parse_mode::root;
            }},
        {"[DirectionalLight]",
            [](scene::configuration& /* config */, const std::string& /* value */)
            {
	        if (mode != parse_mode::root)
	        {
	            LOG_ERROR("Cannot enter directional light parsing from non-root parse mode");
	            return;
	        }
	        mode = parse_mode::directional_light;
            }},
        {"[/DirectionalLight]",
            [](scene::configuration& /* config */, const std::string& /* value */)
            {
	        if (mode != parse_mode::directional_light)
	        {
	            LOG_ERROR("Cannot leave directional_light parsing from non-directional_light parse mode. Potential mismatched tag?");
	            return;
	        }
	        mode = parse_mode::root;
            }},
        {"[Skybox]",
            [](scene::configuration& /* config */, const std::string& /* value */)
            {
	        if (mode != parse_mode::root)
	        {
	            LOG_ERROR("Cannot enter skybox parsing from non-root parse mode");
	            return;
	        }
	        mode = parse_mode::skybox;
            }},
        {"[/Skybox]",
            [](scene::configuration& /* config */, const std::string& /* value */)
            {
	        if (mode != parse_mode::skybox)
	        {
	            LOG_ERROR("Cannot leave skybox parsing from non-directional_light parse mode. Potential mismatched tag?");
	            return;
	        }
	        mode = parse_mode::root;
            }},
        {"[PointLight]",
            [](scene::configuration& /* config */, const std::string& /* value */)
            {
	        if (mode != parse_mode::root)
	        {
	            LOG_ERROR("Cannot enter point light parsing from non-root parse mode");
	            return;
	        }
	        mode = parse_mode::point_light;
	        current_point_light = {};
            }},
        {"[/PointLight]",
            [](scene::configuration& config, const std::string& /* value */)
            {
	        if (mode != parse_mode::point_light)
	        {
	            LOG_ERROR("Cannot leave point light parsing from non-directional_light parse mode. Potential mismatched tag?");
	            return;
	        }
	        mode = parse_mode::root;
	        config.point_lights.push_back(current_point_light);
            }},
        {"[Mesh]",
            [](scene::configuration& /* config */, const std::string& /* value */)
            {
	        if (mode != parse_mode::root)
	        {
	            LOG_ERROR("Cannot enter mesh parsing from non-root parse mode");
	            return;
	        }
	        mode = parse_mode::mesh;
	        current_mesh = {};
            }},
        {"[/Mesh]",
            [](scene::configuration& config, const std::string& /* value */)
            {
	        if (mode != parse_mode::mesh)
	        {
	            LOG_ERROR("Cannot leave mesh parsing from non-directional_light parse mode. Potential mismatched tag?");
	            return;
	        }
	        mode = parse_mode::root;
	        config.meshes.push_back(current_mesh);
            }},
        {"[Terrain]",
            [](scene::configuration& /* config */, const std::string& /* value */)
            {
	        if (mode != parse_mode::root)
	        {
	            LOG_ERROR("Cannot enter terrain parsing from non-root parse mode");
	            return;
	        }
	        mode = parse_mode::terrain;
	        current_terrain = {};
            }},
        {"[/Terrain]",
            [](scene::configuration& config, const std::string& /* value */)
            {
	        if (mode != parse_mode::terrain)
	        {
	            LOG_ERROR("Cannot leave terrain parsing from non-directional_light parse mode. Potential mismatched tag?");
	            return;
	        }
	        mode = parse_mode::root;
	        config.terrains.push_back(current_terrain);
            }},
        {"name",
            [](scene::configuration& configuration, const std::string& value)
            {
	        switch (mode)
	        {
	            using enum egkr::parse_mode;
	        case scene:
	            configuration.name = value;
	            break;
	        case directional_light:
	            configuration.directional_light.name = value;
	            break;
	        case point_light:
	            current_point_light.name = value;
	            break;
	        case skybox:
	            configuration.skybox.name = value;
	            break;
	        case mesh:
	            current_mesh.name = value;
	            break;
	        case terrain:
	            current_terrain.name = value;
	            break;
	        default:
	            break;
	        }
            }},
        {"description",
            [](scene::configuration& configuration, const std::string& value)
            {
	        switch (mode)
	        {
	        case parse_mode::scene:
	            configuration.description = value;
	            break;
	        default:
	            break;
	        }
            }},
        {"resource_name",
            [](scene::configuration& configuration, const std::string& value)
            {
	        switch (mode)
	        {
	        case parse_mode::skybox:
	            configuration.skybox.resource_name = value;
	            break;
	        case parse_mode::mesh:
	            current_mesh.resource_name = value;
	            break;
	        case parse_mode::terrain:
	            current_terrain.resource_name = value;
	            break;
	        default:
	            break;
	        }
            }},
        {"colour",
            [](scene::configuration& configuration, const std::string& value)
            {
	        std::istringstream ss{value};
	        egkr::float4 colour;
	        ss >> colour.r >> colour.g >> colour.b >> colour.a;
	        switch (mode)
	        {
	        case parse_mode::directional_light:
	            configuration.directional_light.colour = colour;
	            break;
	        case parse_mode::point_light:
	            current_point_light.colour = colour;
	            break;
	        default:
	            break;
	        }
            }},
        {"parent", [](scene::configuration& /* configuration */, const std::string& value) { current_mesh.parent_name = value; }},
        {"constant", [](scene::configuration& /* configuration */, const std::string& value) { current_point_light.constant = std::stof(value); }},
        {"linear", [](scene::configuration& /* configuration */, const std::string& value) { current_point_light.linear = std::stof(value); }},
        {"quadratic", [](scene::configuration& /* configuration */, const std::string& value) { current_point_light.quadratic = std::stof(value); }},
        {"transform",
            [](scene::configuration& /* configuration */, const std::string& value)
            {
	        std::istringstream ss{value};
	        ss >> current_mesh.pos.x >> current_mesh.pos.y >> current_mesh.pos.z;

	        ss >> current_mesh.euler_angles.x >> current_mesh.euler_angles.y >> current_mesh.euler_angles.z;

	        current_mesh.euler_angles = glm::radians(current_mesh.euler_angles);
	        ss >> current_mesh.scale.x >> current_mesh.scale.y >> current_mesh.scale.z;
            }},
        {"direction",
            [](scene::configuration& configuration, const std::string& value)
            {
	        std::istringstream ss{value};
	        egkr::float4 direction{};
	        ss >> direction.x >> direction.y >> direction.z >> direction.w;
	        configuration.directional_light.direction = direction;
            }},
        {"position", [](scene::configuration& /* configuration */, const std::string& value)
            {
	        std::istringstream ss{value};
	        egkr::float4 position{};
	        ss >> position.x >> position.y >> position.z >> position.w;

	        current_point_light.position = position;
            }}};

    scene::configuration load_configuration_file(file_handle& handle)
    {
	scene::configuration configuration{};


	uint32_t line_number{};
	egkr::vector<uint8_t> line{'#'};
	for (; !line.empty(); line = filesystem::read_line(handle, 511), ++line_number)
	{
	    std::string line_string{line.begin(), line.end()};
	    trim(line_string);

	    if (line[0] == '#' || line[0] == '\0')
	    {
		continue;
	    }

	    std::string variable_name{};
	    std::string value{};
	    if (line[0] == '[')
	    {
		variable_name = line_string;
	    }
	    else
	    {
		auto split_index = line_string.find_first_of('=');
		if (split_index == std::string::npos)
		{
		    LOG_WARN("Potential formatting issue found in file {}: '=' token not found on line number {}.", handle.filepath.data(), line_number);
		    continue;
		}

		variable_name = line_string.substr(0, split_index);
		trim(variable_name);

		value = line_string.substr(split_index + 1);
		trim(value);
	    }
	    if (!scene_configuration_members.contains(variable_name))
	    {
		LOG_ERROR("Unrecognised scene configuration member: {}", variable_name);
		continue;
	    }

	    scene_configuration_members.at(variable_name)(configuration, value);
	}
	return configuration;
    }
}
