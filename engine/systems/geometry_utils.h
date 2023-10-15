#pragma once
#include "pch.h"
#include "renderer/vertex_types.h"

static void generate_tangents(void* vertices, const egkr::vector<uint32_t>& indices)
{
	auto* verts = (vertex_3d*)vertices;
	for (auto i{ 0U }; i < indices.size(); i += 3)
	{
		auto& vertex1 = verts[indices[i + 0]];
		auto& vertex2 = verts[indices[i + 1]];
		auto& vertex3 = verts[indices[i + 2]];

		auto edge1 = vertex2.position - vertex1.position;
		auto edge2 = vertex3.position - vertex1.position;

		auto delta1 = vertex2.tex - vertex1.tex;
		auto delta2 = vertex3.tex - vertex1.tex;

		auto fc = 1.F / (delta1.x * delta2.y - delta2.x * delta1.y);

		egkr::float3 tangent{};
		tangent.x = fc * (delta2.y * edge1.x - delta1.y * edge2.x);
		tangent.y = fc * (delta2.y * edge1.y - delta1.y * edge2.y);
		tangent.z = fc * (delta2.y * edge1.z - delta1.y * edge2.z);

		tangent = glm::normalize(tangent);

		auto sx = delta1.x;
		auto sy = delta2.x;
		auto tx = delta1.y;
		auto ty = delta2.y;

		auto handedness = (tx * sy - ty * sx) < 0 ? -1 : 1;

		tangent *= handedness;

		vertex1.tangent = { tangent, 1.F };
		vertex2.tangent = { tangent, 1.F };
		vertex3.tangent = { tangent, 1.F };
	}
}

	//static void generate_normals(egkr::vector<void*>&vertices, const egkr::vector<uint32_t>&indices)
	//{
	//	for (auto i{ 0U }; i < indices.size(); i += 3)
	//	{
	//		auto vertex1 = (vertex_3d*)vertices[i + 0];
	//		auto vertex2 = (vertex_3d*)vertices[i + 1];
	//		auto vertex3 = (vertex_3d*)vertices[i + 2];

	//		auto edge1 = vertex2->position - vertex1->position;
	//		auto edge2 = vertex3->position - vertex1->position;

	//		auto normal = glm::normalize(glm::cross(edge1, edge2));

	//		vertex1->normal = normal;
	//		vertex2->normal = normal;
	//		vertex3->normal = normal;
	//	}
	//}
