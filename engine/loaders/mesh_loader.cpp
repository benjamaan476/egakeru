#include "mesh_loader.h"
#include <filesystem>
#include "systems/geometry_utils.h"

namespace egkr
{
	mesh_loader::unique_ptr mesh_loader::create(const loader_properties& properties)
	{
		return std::make_unique<mesh_loader>(properties);
	}

	mesh_loader::mesh_loader(const loader_properties& properties)
		:resource_loader{ resource_type::mesh, properties } {}

	resource::shared_ptr mesh_loader::load(std::string_view name)
	{
		egkr::vector<supported_file_type> file_types{/*{".esm", mesh_file_type::esm, true},*/ { ".obj", mesh_file_type::obj, false }};

		bool found{};
		supported_file_type found_type{};
		char filename[128];
		for (const auto& file_type : file_types)
		{
			const auto base_path = get_base_path();
			constexpr std::string_view format_string{ "%s/%s%s" };

			sprintf_s(filename, format_string.data(), base_path.data(), name.data(), file_type.extension.data());
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

		egkr::vector<geometry_properties> resource_data{};

		switch (found_type.file_type)
		{
		case mesh_file_type::obj:
			resource_data = import_obj(handle, name.data());
			break;
		case mesh_file_type::esm:
			resource_data = load_esm(handle);
			break;
		default:
			LOG_ERROR("Invalid file type, cannot load {}", name.data());
			return nullptr;
		}

		resource_properties properties{};
		properties.name = name;
		properties.full_path = filename;
		properties.type = resource_type::mesh;

		properties.data = new egkr::vector<geometry_properties>();
		*(egkr::vector<geometry_properties>*)properties.data = resource_data;
		return std::make_shared<resource>(properties);
	}

	bool mesh_loader::unload(const resource::shared_ptr& resource)
	{
		auto* data = (egkr::vector<geometry_properties>*)resource->data;
		data->clear();
		delete data;
		return false;
	}

	egkr::vector<geometry_properties> mesh_loader::import_obj(file_handle& file_handle, std::string_view esm_filename)
	{
		egkr::vector<geometry_properties> geometries{};

		egkr::vector<float3> positions{};
		positions.reserve(16384);

		egkr::vector<float3> normals{};
		normals.reserve(16384);

		egkr::vector<float2> tex_coords{};
		tex_coords.reserve(16384);

		egkr::vector<mesh_group_data> groups{1};
		uint32_t group_id{};
		groups.reserve(4);

		std::string name{};
		std::string material_filename{};
		//uint32_t current_material_name_count{};
		egkr::vector<std::string> material_names{32};

		std::array<char, 2> previous_first_chars{};

		while (true)
		{
			auto line = filesystem::read_line(file_handle, 512);
			if (line.empty())
			{
				break;
			}

			std::string line_string{ line.begin(), line.end() };
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
					sscanf_s(line_string.data(), "%s %f %f %f", waste, 2, &pos.x, &pos.y, &pos.z);
					positions.push_back(pos);
				}
				break;
				case 'n':
				{
					char waste[3]{};
					float3 norm{};
					sscanf_s(line_string.data(), "%s %f %f %f", waste, 3, &norm.x, &norm.y, &norm.z);
					normals.push_back(norm);
				}
				break;
				case 't':
				{
					char waste[3]{};
					float2 tex{};
					sscanf_s(line_string.data(), "%s %f %f", waste, 3, &tex.x, &tex.y);
					tex_coords.push_back(tex);
				}
				break;
				}
			}
			case 's':
				break;
			case 'f':
			{
				char waste[2]{};
				mesh_face_data face{};
				if (normals.empty() || tex_coords.empty())
				{
					std::string format{"%s %d %d %d"};
					sscanf_s(line_string.data(), format.data(), waste, 2, &face.vertices[0].position_index, &face.vertices[1].position_index, &face.vertices[2].position_index);
				}
				else
				{
					std::string format{"%s %d/%d/%d %d/%d/%d %d/%d/%d"};
					sscanf_s(line_string.data(), format.data(), waste, 2, &face.vertices[0].position_index, &face.vertices[0].tex_index, &face.vertices[0].normal_index, &face.vertices[1].position_index, &face.vertices[1].tex_index, &face.vertices[1].normal_index, &face.vertices[2].position_index, &face.vertices[2].tex_index, &face.vertices[2].normal_index);
				}

				groups[group_id].faces.push_back(face);
			}
			break;
			case 'm':
			{
				char waste[7]{};
				char matname[128]{};
				std::string format{"%s %s"};
				sscanf_s(line_string.data(), format.data(), waste, 7, matname, 128);
				material_filename = matname;
			}
			break;
			case 'u':
			{
				mesh_group_data new_group{};
				new_group.faces.reserve(16384);
				groups.push_back(new_group);
				++group_id;
				material_names.push_back(material_filename);
				//current_material_name_count++;
			}
			break;
			case 'g':
			{
				auto group_count = groups.size();
				for (auto i{ 0U }; i < group_count; ++i)
				{
					geometry_properties properties{};
					properties.name = name;

					if (i > 0)
					{
						properties.name += std::to_string(i);
					}
					properties.material_name = material_names[i];
					auto geometry_properties = process_subobject(positions, normals, tex_coords, groups[i].faces);
					geometries.push_back(geometry_properties);
				}
				groups.clear();

				char waste[2]{};
				char names[128]{};
				sscanf_s(line_string.data(), "%s %s", waste, 2, names, 128);
				name = names;

			}
			break;
			}

			previous_first_chars[1] = previous_first_chars[0];
			previous_first_chars[0] = first_char;
			
		}

		auto group_count = groups.size();
		for (auto i{ 0U }; i < group_count; ++i)
		{
			geometry_properties properties{};
			properties.name = name;

			if (i > 0)
			{
				properties.name += std::to_string(i);
			}
			properties.material_name = material_names[i];
			auto geometry_properties = process_subobject(positions, normals, tex_coords, groups[i].faces);
			geometries.push_back(std::move(geometry_properties));
		}

		if (!material_filename.empty())
		{
			std::filesystem::path full_material_path{esm_filename.data()};
			auto root_dir = full_material_path.root_directory();
			root_dir += material_filename;

			if (!import_obj_material_library(root_dir.string()))
			{
				LOG_ERROR("Failed to load obj mtl");
			}
		}

		for (auto& geometry : geometries)
		{
			auto unique_verts = deduplicate_vertices(geometry.vertex_count, (vertex_3d*)geometry.vertices, geometry.indices);

			delete (vertex_3d*)geometry.vertices;

			auto size = geometry.vertex_count * geometry.vertex_size;
			geometry.vertices = malloc(size);
			auto* new_verts = (vertex_3d*)geometry.vertices;

			std::copy(unique_verts.data(), unique_verts.data() + geometry.vertex_count, new_verts);
		}

		write_esm(esm_filename, geometries);
		return geometries;
	}

	geometry_properties mesh_loader::process_subobject(egkr::vector<float3>& positions, const egkr::vector<float3>& normals, const egkr::vector<float2>& tex, egkr::vector<mesh_face_data> faces)
	{
		geometry_properties properties{};
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

		for (auto f{ 0U }; f < faces.size(); ++f)
		{
			const auto& face = faces[f];
			for (auto i{ 0U }; i < face.vertices.size(); ++i)
			{
				const auto& index_data = face.vertices[i];
				properties.indices.push_back(i + 3 * f);

				vertex_3d vertex{};
				vertex.position = positions[index_data.position_index - 1];

				if (vertex.position.x < properties.min_extent.x || !extent_set)
				{
					properties.min_extent.x = vertex.position.x;
				}
				if (vertex.position.y < properties.min_extent.y || !extent_set)
				{
					properties.min_extent.y = vertex.position.y;
				}
				if (vertex.position.z < properties.min_extent.z || !extent_set)
				{
					properties.min_extent.z = vertex.position.z;
				}

				if (vertex.position.x > properties.max_extent.x || !extent_set)
				{
					properties.max_extent.x = vertex.position.x;
				}
				if (vertex.position.y > properties.max_extent.y || !extent_set)
				{
					properties.max_extent.y = vertex.position.y;
				}
				if (vertex.position.z > properties.max_extent.z || !extent_set)
				{
					properties.max_extent.z = vertex.position.z;
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

		properties.vertex_count = vertices.size();
		properties.vertex_size = sizeof(vertex_3d);

		auto size = properties.vertex_count * properties.vertex_size;

		properties.vertices = malloc(size);

		std::copy(vertices.data(), vertices.data() + properties.vertex_count, (vertex_3d*)properties.vertices);

		properties.center = (properties.min_extent + properties.max_extent) / 2.F;

		generate_tangents(properties.vertices, properties.indices);

		return properties;
	}

	bool mesh_loader::import_obj_material_library(std::string_view /*filepath*/)
	{
		return false;
	}

	egkr::vector<geometry_properties> mesh_loader::load_esm(const file_handle& /*file_handle*/)
	{
		return egkr::vector<geometry_properties>();
	}

	bool mesh_loader::write_esm(std::string_view /*path*/, const egkr::vector<geometry_properties>& /*properties*/)
	{
		return true;
	}

	bool mesh_loader::write_emt(std::string_view /*directory*/, const material_properties& /*properties*/)
	{
		return true;
	}
}