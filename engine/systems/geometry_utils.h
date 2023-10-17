#pragma once
#include "pch.h"
#include "renderer/vertex_types.h"

static void generate_tangents(void* verts, const egkr::vector<uint32_t>& indices)
{
	auto* vertices = (vertex_3d*)verts;
	for (auto i{ 0U }; i < indices.size(); i += 3)
	{
        auto i0 = indices[i + 0];
        auto i1 = indices[i + 1];
        auto i2 = indices[i + 2];

        auto edge1 = vertices[i1].position - vertices[i0].position;
        auto edge2 = vertices[i2].position - vertices[i0].position;

        auto deltaU1 = vertices[i1].tex.x - vertices[i0].tex.x;
        auto deltaV1 = vertices[i1].tex.y - vertices[i0].tex.y;

        auto deltaU2 = vertices[i2].tex.x - vertices[i0].tex.x;
        auto deltaV2 = vertices[i2].tex.y - vertices[i0].tex.y;

        auto dividend = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
        auto fc = 1.0f / dividend;

        auto tangent = (egkr::float3){
            (fc * (deltaV2 * edge1.x - deltaV1 * edge2.x)),
            (fc * (deltaV2 * edge1.y - deltaV1 * edge2.y)),
            (fc * (deltaV2 * edge1.z - deltaV1 * edge2.z)) };

        tangent = glm::normalize(tangent);

        auto sx = deltaU1, sy = deltaU2;
        auto tx = deltaV1, ty = deltaV2;
        auto handedness = ((tx * sy - ty * sx) < 0.0f) ? -1.0f : 1.0f;
        egkr::float4 t4 = { tangent, handedness };
        vertices[i0].tangent = t4;
        vertices[i1].tangent = t4;
        vertices[i2].tangent = t4;
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

static void reassign_index(uint32_t index_count, uint32_t* indices, uint32_t from, uint32_t to)
{
    for (auto i{ 0U }; i < index_count; ++i)
    {
        if (indices[i] == from)
        {
            indices[i] = to;
        }
        else if (indices[i] > from)
        {
            indices[i]--;
        }
    }
}

inline static egkr::vector<vertex_3d> deduplicate_vertices(uint32_t vertex_count, vertex_3d* vertices, egkr::vector<uint32_t>& indices)
{
    egkr::vector<vertex_3d> new_vertices{vertex_count};
    uint32_t found_count{};

    uint32_t out_vert_count{};
    for (auto v{ 0U }; v < vertex_count; ++v)
    {
        bool found{};
        for (auto u{ 0U }; u < out_vert_count; ++u)
        {
            if (vertices[v] == vertices[u])
            {
                reassign_index(indices.size(), indices.data(), v - found_count, u);
                found = true;
                ++found_count;
                break;
            }
        }

        if (!found)
        {
            new_vertices[out_vert_count] = vertices[v];
            out_vert_count++;
        }
    }

    return { new_vertices.begin(), new_vertices.begin() + out_vert_count };
}