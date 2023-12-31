#include "mesh.h"

namespace egkr
{
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
		for (const auto& geom : geometries_)
		{
			geom->free();
		}
	}

	void mesh::add_geometry(const geometry::geometry::shared_ptr& geometry)
	{
		geometries_.push_back(geometry);
	}

	void mesh::set_model(const transform& model)
	{
		model_ = model;
	}
}