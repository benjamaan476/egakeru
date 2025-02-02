#include "mesh_loader.h"
#include <filesystem>
#include "systems/geometry_utils.h"

namespace egkr
{
    mesh_loader::unique_ptr mesh_loader::create(const loader_properties& properties) { return std::make_unique<mesh_loader>(properties); }

    mesh_loader::mesh_loader(const loader_properties& properties): resource_loader{resource::type::mesh, properties} { }

    resource::shared_ptr mesh_loader::load(const std::string& name, void* /*params*/)
    {
	egkr::vector<supported_file_type> file_types{{.extension = ".esm", .file_type = mesh_file_type::esm, .is_binary = true}, {.extension = ".obj", .file_type = mesh_file_type::obj, .is_binary = false}};

	bool found{};
	supported_file_type found_type{};
	std::string filename;
	for (const auto& file_type : file_types)
	{
	    const auto base_path = get_base_path();
	    filename = std::format("{}/{}{}", base_path, name, file_type.extension);

	    auto handle = filesystem::open(filename, file_mode::read, file_type.is_binary);
	    if (handle.is_valid)
	    {
		found = true;
		found_type = file_type;

		break;
	    }
	    filesystem::close(handle);
	}

	auto handle = filesystem::open(filename, file_mode::read, found_type.is_binary);

	if (!found)
	{
	    LOG_ERROR("Could not find mesh file: {}", name.data());
	    return nullptr;
	}

	egkr::vector<geometry::properties> resource_data{};

	switch (found_type.file_type)
	{
	case mesh_file_type::obj:
	    resource_data = import_obj(handle, filename);
	    break;
	case mesh_file_type::esm:
	    resource_data = load_esm(handle);
	    break;
	case mesh_file_type::not_found:
	default:
	    LOG_ERROR("Invalid file type, cannot load {}", name.data());
	    return nullptr;
	}

	resource::properties properties{};
	properties.name = name;
	properties.full_path = filename;
	properties.type = resource::type::mesh;

	properties.data = new egkr::vector<geometry::properties>();
	*(egkr::vector<geometry::properties>*)properties.data = resource_data;
	return resource::create(properties);
    }

    bool mesh_loader::unload(const resource::shared_ptr& resource)
    {
	auto* data = (egkr::vector<geometry::properties>*)resource->data;
	for (auto& geo : *data)
	{
	    free(geo.vertices);
	}
	data->clear();
	delete data;
	return false;
    }

    egkr::vector<geometry::properties> mesh_loader::import_obj(file_handle& file_handle, std::string_view esm_filename)
    {
	egkr::vector<geometry::properties> geometries{};

	egkr::vector<float3> positions{};
	positions.reserve(16384);

	egkr::vector<float3> normals{};
	normals.reserve(16384);

	egkr::vector<float2> tex_coords{};
	tex_coords.reserve(16384);

	egkr::vector<mesh_group_data> groups{1};
	groups.reserve(4);

	std::string name{};
	std::string material_filename{};
	uint32_t current_material_name_count{};
	egkr::vector<std::string> material_names{32};

	std::array<char, 2> previous_first_chars{};

	while (true)
	{
	    auto line = filesystem::read_line(file_handle, 512);
	    if (line.empty())
	    {
		break;
	    }

	    std::string line_string{line.begin(), line.end()};
	    trim(line_string);


	    uint8_t first_char = line[0];

	    switch (first_char)
	    {
	    case '#':
		continue;
	    case 'v':
	    {
		auto second_char = line[1];
		switch (second_char)
		{
		case ' ':
		{
		    char waste[2]{};
		    float3 pos{};
		    sscanf(line_string.data(), "%s %f %f %f", waste, &pos.x, &pos.y, &pos.z);
		    positions.push_back(pos);
		}
		break;
		case 'n':
		{
		    char waste[3]{};
		    float3 norm{};
		    sscanf(line_string.data(), "%s %f %f %f", waste, &norm.x, &norm.y, &norm.z);
		    normals.push_back(norm);
		}
		break;
		case 't':
		{
		    char waste[3]{};
		    float2 tex{};
		    sscanf(line_string.data(), "%s %f %f", waste, &tex.x, &tex.y);
		    tex_coords.push_back(tex);
		}
		break;
		default:
		    LOG_ERROR("Shouldn't have got here");
		}
	    }
	    break;
	    case 's':
		break;
	    case 'f':
	    {
		char waste[2]{};
		mesh_face_data face{};
		if (normals.empty() || tex_coords.empty())
		{
		    sscanf(line_string.data(), "%s %u %u %u", waste, &face.vertices[0].position_index, &face.vertices[1].position_index, &face.vertices[2].position_index);
		}
		else
		{
		    sscanf(line_string.data(), "%s %u/%u/%u %u/%u/%u %u/%u/%u", waste, &face.vertices[0].position_index, &face.vertices[0].tex_index, &face.vertices[0].normal_index,
		        &face.vertices[1].position_index, &face.vertices[1].tex_index, &face.vertices[1].normal_index, &face.vertices[2].position_index, &face.vertices[2].tex_index,
		        &face.vertices[2].normal_index);
		}

		auto group_index = groups.size() - 1;
		groups[group_index].faces.push_back(face);
	    }
	    break;
	    case 'm':
	    {
		char waste[7]{};
		char matname[128]{};
		sscanf(line_string.data(), "%s %s", waste, matname);
		material_filename = matname;
	    }
	    break;
	    case 'u':
	    {
		mesh_group_data new_group{};
		new_group.faces.reserve(16384);
		groups.push_back(new_group);

		char waste[8]{};
		char material[128]{};
		sscanf(line_string.data(), "%s %s", waste, material);
		material_names[current_material_name_count] = material;
		current_material_name_count++;
	    }
	    break;
	    case 'g':
	    {
		auto group_count = groups.size();
		for (auto i{0U}; i < group_count; ++i)
		{
		    auto geometry_properties = process_subobject(positions, normals, tex_coords, groups[i].faces);
		    geometry_properties.name = name;
		    if (geometry_properties.vertex_count == 0)
		    {
			LOG_WARN("Geometry generated with no vertices. Skipping");
			continue;
		    }

		    if (i > 0)
		    {
			geometry_properties.name += std::to_string(i);
		    }
		    geometry_properties.material_name = material_names[i];
		    geometries.push_back(geometry_properties);
		    material_names[i].clear();
		}
		current_material_name_count = 0;
		groups.clear();
		char waste[2]{};
		char names[128]{};
		sscanf(line_string.data(), "%s %s", waste, names);
		name = names;
	    }
	    break;
	    default:
		LOG_ERROR("Unrecognised first character: {}", first_char);
		break;
	    }
	    previous_first_chars[1] = previous_first_chars[0];
	    previous_first_chars[0] = (char)first_char;
	}

	auto group_count = groups.size();
	for (auto i{0U}; i < group_count; ++i)
	{
	    auto geometry_properties = process_subobject(positions, normals, tex_coords, groups[i].faces);

	    if (geometry_properties.vertex_count == 0)
	    {
		LOG_WARN("Geometry generated with no vertices. Skipping");
		continue;
	    }
	    geometry_properties.name = name;

	    if (i > 0)
	    {
		geometry_properties.name += std::to_string(i);
	    }
	    geometry_properties.material_name = material_names[i];
	    geometries.push_back(std::move(geometry_properties));
	}

	if (!material_filename.empty())
	{
	    std::filesystem::path full_material_path{esm_filename.data()};
	    full_material_path = std::filesystem::absolute(full_material_path);
	    auto root_dir = full_material_path.parent_path();
	    root_dir += "\\" + material_filename;

	    if (!import_obj_material_library(root_dir.string()))
	    {
		LOG_ERROR("Failed to load obj mtl");
	    }
	}

	for (auto& geometry : geometries)
	{
	    auto unique_verts = deduplicate_vertices(geometry.vertex_count, (vertex_3d*)geometry.vertices, geometry.indices);
	    geometry.vertex_count = (uint32_t)unique_verts.size();

	    delete (vertex_3d*)geometry.vertices;

	    auto size = geometry.vertex_count * geometry.vertex_size;
	    geometry.vertices = malloc(size);

	    std::copy(unique_verts.data(), unique_verts.data() + geometry.vertex_count, (vertex_3d*)geometry.vertices);
	}

	std::filesystem::path esm{esm_filename};
	esm.replace_extension();

	write_esm(esm.string(), geometries);
	return geometries;
    }

    geometry::properties mesh_loader::process_subobject(egkr::vector<float3>& positions, const egkr::vector<float3>& normals, const egkr::vector<float2>& tex, egkr::vector<mesh_face_data> faces)
    {
	geometry::properties properties{};
	properties.material_name = "default";
	egkr::vector<vertex_3d> vertices;
	bool extent_set{};
	bool skip_normals{};
	if (normals.empty())
	{
	    skip_normals = true;
	}

	bool skip_textures{};
	if (tex.empty())
	{
	    skip_textures = true;
	}

	for (auto f{0U}; f < faces.size(); ++f)
	{
	    const auto& face = faces[f];
	    for (auto i{0U}; i < face.vertices.size(); ++i)
	    {
		const auto& index_data = face.vertices[i];
		properties.indices.push_back(i + 3 * f);

		vertex_3d vertex{};
		vertex.position = positions[index_data.position_index - 1];

		if (vertex.position.x < properties.extents.min.x || !extent_set)
		{
		    properties.extents.min.x = vertex.position.x;
		}
		if (vertex.position.y < properties.extents.min.y || !extent_set)
		{
		    properties.extents.min.y = vertex.position.y;
		}
		if (vertex.position.z < properties.extents.min.z || !extent_set)
		{
		    properties.extents.min.z = vertex.position.z;
		}

		if (vertex.position.x > properties.extents.max.x || !extent_set)
		{
		    properties.extents.max.x = vertex.position.x;
		}
		if (vertex.position.y > properties.extents.max.y || !extent_set)
		{
		    properties.extents.max.y = vertex.position.y;
		}
		if (vertex.position.z > properties.extents.max.z || !extent_set)
		{
		    properties.extents.max.z = vertex.position.z;
		}

		extent_set = true;

		if (!skip_normals)
		{
		    vertex.normal = normals[index_data.normal_index - 1];
		}

		if (!skip_textures)
		{
		    vertex.tex = tex[index_data.tex_index - 1];
		}

		vertices.push_back(vertex);
	    }
	}

	properties.vertex_count = (uint32_t)vertices.size();
	properties.vertex_size = sizeof(vertex_3d);

	auto size = properties.vertex_count * properties.vertex_size;

	properties.vertices = malloc(size);

	generate_tangents(vertices, properties.indices);
	std::copy(vertices.data(), vertices.data() + properties.vertex_count, (vertex_3d*)properties.vertices);

	properties.center = (properties.extents.min + properties.extents.max) / 2.F;


	return properties;
    }

    bool mesh_loader::import_obj_material_library(std::string_view filepath)
    {
	auto file = filesystem::open(filepath, file_mode::read, false);
	material::properties current_properties{};

	bool hit_name{};
	while (true)
	{
	    auto line = filesystem::read_line(file, 511);
	    if (line.empty())
	    {
		break;
	    }

	    std::string line_string{line.begin(), line.end()};
	    trim(line_string);

	    auto first_char = line_string[0];
	    switch (first_char)
	    {
	    case '#':
		continue;
	    case 'K':
	    {
		auto second_char{line_string[1]};
		switch (second_char)
		{
		case 'a':
		case 'd':
		{
		    char waste[3]{};
		    sscanf(line_string.data(), "%s %f %f %f", waste, &current_properties.diffuse_colour.r, &current_properties.diffuse_colour.g, &current_properties.diffuse_colour.b);
		    current_properties.diffuse_colour.a = 1.F;
		}
		break;
		case 's':
		    break;
		default:
		    LOG_ERROR("Unrecognised second character: {}", second_char);
		}
	    }
	    break;
	    case 'N':
	    {
		char waste[3]{};
		sscanf(line_string.data(), "%s %f", waste, &current_properties.shininess);
	    }
	    break;
	    case 'm':
	    {
		char map_type[10]{};
		char texture_filename[128]{};
		sscanf(line_string.data(), "%s %s", map_type, texture_filename);

		std::filesystem::path path{texture_filename};
		path = path.stem();
		path.replace_extension();
		if (strncmp(map_type, "map_Kd", 7) == 0)
		{
		    current_properties.diffuse_map_name = path.string();
		}
		else if (strncmp(map_type, "map_Ks", 7) == 0)
		{
		    current_properties.specular_map_name = path.string();
		}
		else if (strncmp(map_type, "map_bump", 9) == 0)
		{
		    current_properties.normal_map_name = path.string();
		}
	    }
	    break;
	    case 'b':
	    {
		char map_type[10]{};
		char texture_filename[128]{};
		sscanf(line_string.data(), "%s %s", map_type, texture_filename);

		std::filesystem::path path{texture_filename};
		path = path.stem();
		path.replace_extension();
		current_properties.normal_map_name = path.string();
	    }
	    break;
	    case 'n':
	    {
		char waste[7]{};
		char material_name[128]{};
		sscanf(line_string.data(), "%s %s", waste, material_name);

		current_properties.shader_name = "Shader.Material";
		if (std::abs(current_properties.shininess) <= 0.001f)
		{
		    current_properties.shininess = 8.F;
		}

		if (hit_name)
		{
		    std::filesystem::path full_material_path{filepath};
		    full_material_path = std::filesystem::absolute(full_material_path);
		    auto root_dir = full_material_path.parent_path();
		    write_emt(root_dir.string(), current_properties);
		}
		hit_name = true;
		current_properties.name = material_name;
	    }
	    break;
	    default:
		LOG_ERROR("Unrecognised first character: {}", first_char);
	    }
	}

	current_properties.shader_name = "Shader.Material";
	if (std::abs(current_properties.shininess) <= 0.001F)
	{
	    current_properties.shininess = 8.F;
	}

	if (hit_name)
	{
	    std::filesystem::path full_material_path{filepath};
	    full_material_path = std::filesystem::absolute(full_material_path);
	    auto root_dir = full_material_path.parent_path();
	    write_emt(root_dir.string(), current_properties);
	}
	return true;
    }

    egkr::vector<geometry::properties> mesh_loader::load_esm(file_handle& file_handle)
    {
	if (!file_handle.is_valid)
	{
	    LOG_ERROR("Invalid file handle used");
	    return {};
	}

	size_t property_count{};
	filesystem::read(file_handle, &property_count, 1);

	egkr::vector<geometry::properties> geoms;
	for (auto i{0U}; i < property_count; ++i)
	{
	    geometry::properties property{};

	    size_t name_length{};
	    filesystem::read(file_handle, &name_length, 1);

	    property.name.resize(name_length);
	    filesystem::read(file_handle, property.name.data(), name_length);

	    //property.name = std::string(name);

	    size_t material_name_length{};
	    filesystem::read(file_handle, &material_name_length, 1);

	    property.material_name.resize(material_name_length);
	    filesystem::read(file_handle, property.material_name.data(), material_name_length);

	    filesystem::read(file_handle, &property.vertex_count, 1);
	    filesystem::read(file_handle, &property.vertex_size, 1);

	    auto size = property.vertex_count * property.vertex_size;
	    property.vertices = malloc(size);

	    filesystem::read(file_handle, property.vertices, property.vertex_size, property.vertex_count);
	    //std::copy((uint8_t*)property.vertices, (uint8_t*)property.vertices + size, verts.data());

	    size_t index_count{};
	    filesystem::read(file_handle, &index_count, 1);

	    property.indices.resize(index_count);
	    filesystem::read(file_handle, property.indices.data(), index_count);

	    filesystem::read(file_handle, &property.center, 1);
	    filesystem::read(file_handle, &property.extents.min, 1);
	    filesystem::read(file_handle, &property.extents.max, 1);
	    geoms.push_back(property);
	}

	return geoms;
    }

    bool mesh_loader::write_esm(std::string_view path, const egkr::vector<geometry::properties>& properties)
    {
	auto filename = path.data() + std::string(".esm");
	auto handle = filesystem::open(filename, file_mode::write, true);

	filesystem::write(handle, properties.size(), 1);

	for (const auto& property : properties)
	{
	    filesystem::write(handle, property.name.size(), 1);
	    filesystem::write(handle, property.name.data(), 1, property.name.size());

	    filesystem::write(handle, property.material_name.size(), 1);
	    filesystem::write(handle, property.material_name.data(), 1, property.material_name.size());

	    filesystem::write(handle, property.vertex_count, 1);
	    filesystem::write(handle, property.vertex_size, 1);
	    filesystem::write(handle, property.vertices, property.vertex_size, property.vertex_count);

	    filesystem::write(handle, property.indices.size(), 1);
	    filesystem::write(handle, property.indices.data(), sizeof(uint32_t), property.indices.size());

	    filesystem::write(handle, property.center, 1);
	    filesystem::write(handle, property.extents.min, 1);
	    filesystem::write(handle, property.extents.max, 1);
	}
	return true;
    }

    bool mesh_loader::write_emt(std::string_view directory, const material::properties& properties)
    {
	//TODO: ew
	std::string filename = std::format("{}/../materials/{}{}", directory, properties.name, ".emt");
	auto handle = filesystem::open(filename, file_mode::write, false);

	egkr::vector<uint8_t> line(128);

	sprintf((char*)line.data(), "# %s", properties.name.data());
	filesystem::write_line(handle, line);
	line.clear();

	sprintf((char*)line.data(), "# auto generated material");
	filesystem::write_line(handle, line);
	line.clear();

	sprintf((char*)line.data(), "version = 0.1");
	filesystem::write_line(handle, line);
	line.clear();

	sprintf((char*)line.data(), "name=%s", properties.name.data());
	filesystem::write_line(handle, line);
	line.clear();

	sprintf((char*)line.data(), "diffuse_colour=%.3f %.3f %.3f %.3f", (double)properties.diffuse_colour.r, (double)properties.diffuse_colour.g, (double)properties.diffuse_colour.b,
	    (double)properties.diffuse_colour.a);
	filesystem::write_line(handle, line);
	line.clear();

	sprintf((char*)line.data(), "diffuse_map_name=%s", properties.diffuse_map_name.data());
	filesystem::write_line(handle, line);
	line.clear();

	sprintf((char*)line.data(), "specular_map_name=%s", properties.specular_map_name.data());
	filesystem::write_line(handle, line);
	line.clear();

	sprintf((char*)line.data(), "normal_map_name=%s", properties.normal_map_name.data());
	filesystem::write_line(handle, line);
	line.clear();

	sprintf((char*)line.data(), "shader=%s", properties.shader_name.data());
	filesystem::write_line(handle, line);
	line.clear();

	sprintf((char*)line.data(), "shininess=%.6f", (double)properties.shininess);
	filesystem::write_line(handle, line);
	line.clear();

	return true;
    }
}
