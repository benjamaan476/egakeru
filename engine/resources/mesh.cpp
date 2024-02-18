#include "mesh.h"

#include "systems/geometry_system.h"
#include "systems/resource_system.h"

#include "systems/job_system.h"

namespace egkr
{
	struct mesh_load_parameters
	{
		std::string resource_name{};
		mesh::shared_ptr mesh{};
		resource::resource::shared_ptr mesh_resource{};
	};

	void load_job_success(void* params)
	{
		auto* mesh_params = (mesh_load_parameters*)params;

		auto* geo_configs = (egkr::vector<geometry::properties>*)mesh_params->mesh_resource->data;

		for (const auto& geo : *geo_configs)
		{
			mesh_params->mesh->add_geometry(geometry_system::acquire(geo));
			auto local_extents = geo.extents;
			vertex_3d* vertices = (vertex_3d*)geo.vertices;

			for (uint32_t i{}; i < geo.vertex_count; ++i)
			{
				const auto vertex = vertices[i];
				if (vertex.position.x < local_extents.min.x)
				{
					local_extents.min.x = vertex.position.x;
				}
				if (vertex.position.y < local_extents.min.y)
				{
					local_extents.min.y = vertex.position.y;
				}
				if (vertex.position.z < local_extents.min.z)
				{
					local_extents.min.z = vertex.position.z;
				}
				if (vertex.position.x > local_extents.max.x)
				{
					local_extents.max.x = vertex.position.x;
				}
				if (vertex.position.y > local_extents.max.y)
				{
					local_extents.max.y = vertex.position.y;
				}
				if (vertex.position.z > local_extents.max.z)
				{
					local_extents.max.z = vertex.position.z;
				}
			}

			auto& global_extents = mesh_params->mesh->extents();

				if (local_extents.min.x < global_extents.min.x)
				{
					global_extents.min.x = local_extents.min.x;
				}
				if (local_extents.min.y < global_extents.min.y)
				{
					global_extents.min.y = local_extents.min.y;
				}
				if (local_extents.min.z < global_extents.min.z)
				{
					global_extents.min.z = local_extents.min.z;
				}
				if (local_extents.max.x > global_extents.max.x)
				{
					global_extents.max.x = local_extents.max.x;
				}
				if (local_extents.max.y > global_extents.max.y)
				{
					global_extents.max.y = local_extents.max.y;
				}
				if (local_extents.max.z > global_extents.max.z)
				{
					global_extents.max.z = local_extents.max.z;
				}
		}

		mesh_params->mesh->increment_generation();
		LOG_TRACE("Successfully loaded mesh '{}'", mesh_params->resource_name);

		resource_system::unload(mesh_params->mesh_resource);
	}

	void load_job_fail(void* params)
	{
		auto* mesh_params = (mesh_load_parameters*)params;
		LOG_ERROR("Failed to load mesh '{}'", mesh_params->resource_name);

		resource_system::unload(mesh_params->mesh_resource);
	}

	bool load_job_start(void* params, void* result_data)
	{
		auto* load_params = (mesh_load_parameters*)params;
		auto mesh = resource_system::load(load_params->resource_name, resource_type::mesh, nullptr);
		load_params->mesh_resource = mesh;
		memcpy(result_data, load_params, sizeof(mesh_load_parameters));

		return mesh != nullptr;
	}

	mesh::shared_ptr mesh::create()
	{
		return std::make_shared<mesh>();
	}

	mesh::shared_ptr mesh::create(const geometry::geometry::shared_ptr& geometry, const transform& model)
	{
		auto mesh = create();
		mesh->add_geometry(geometry);
		mesh->set_model(model);

		return mesh;
	}

	mesh::mesh()
		: resource(0, 0, "")
	{

	}

	mesh::~mesh()
	{
		unload();
	}

	void mesh::add_geometry(const geometry::geometry::shared_ptr& geometry)
	{
		geometries_.push_back(geometry);
			auto& global_extents = extents();
			const auto& geo_extents = geometry->get_properties().extents;
				if (geo_extents.min.x < global_extents.min.x)
				{
					global_extents.min.x = geo_extents.min.x;
				}
				if (geo_extents.min.y < global_extents.min.y)
				{
					global_extents.min.y = geo_extents.min.y;
				}
				if (geo_extents.min.z < global_extents.min.z)
				{
					global_extents.min.z = geo_extents.min.z;
				}
				if (geo_extents.max.x > global_extents.max.x)
				{
					global_extents.max.x = geo_extents.max.x;
				}
				if (geo_extents.max.y > global_extents.max.y)
				{
					global_extents.max.y = geo_extents.max.y;
				}
				if (geo_extents.max.z > global_extents.max.z)
				{
					global_extents.max.z = geo_extents.max.z;
				}
	}

	void mesh::set_model(const transform& model)
	{
		model_ = model;
	}

	void mesh::unload()
	{
		if (debug_data)
		{
			debug_data->destroy();
			debug_data.reset();
		}

		for (auto& geo : geometries_)
		{
			geometry_system::release_geometry(geo);
		}

		geometries_.clear();
		set_generation(invalid_32_id);
	}

	mesh::shared_ptr mesh::load(std::string_view name)
	{
		auto mesh = mesh::create();
		mesh_load_parameters parameters{ .resource_name = name.data(), .mesh = mesh, .mesh_resource = {}};

		auto info = job_system::job_system::create_job(load_job_start, load_job_success, load_job_fail, &parameters, sizeof(mesh_load_parameters), sizeof(mesh_load_parameters));
		job_system::job_system::submit(info);

		return mesh;

	}
}