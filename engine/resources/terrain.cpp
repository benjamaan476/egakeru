#include "terrain.h"
#include <cstdlib>
#include <memory>
#include "identifier.h"
#include "pch.h"

namespace egkr
{
    terrain::shared_ptr terrain::create(const terrain::configuration& configuration) { return std::make_shared<terrain>(configuration); }

    terrain::terrain(const terrain::configuration& configuration)
        : resource(0, 0, configuration.name), unique_id{identifier::acquire_unique_id(this)}, name{configuration.name}, tiles_x{configuration.tiles_x}, tiles_y{configuration.tiles_y},
          scale_x{configuration.scale_x}, scale_y{configuration.scale_y}
    {
	vertices.resize(tiles_x * tiles_y);
	indices.resize(6 * tiles_x * tiles_y);

	uint32_t i{0};
	for (uint32_t y{}; y < tiles_y - 1; y++)
	{
	    for (uint32_t x{}; x < tiles_x - 1; x++, i += 6)
	    {
		const uint32_t v0 = y * tiles_x + x;
		const uint32_t v1 = v0 + 1;
		const uint32_t v2 = v0 + tiles_x;
		const uint32_t v3 = v2 + 1;

		vertices[v0] = terrain::vertex{.position = {x * scale_x, y * scale_y, std::rand() % 10}, .normal = {0, 0, 1}, .texture_coords = {x, y}, .colour{0.5, 0.5, 0.5, 1.0}, .tangent{1, 0, 0, 0}};

		indices[i] = v0;
		indices[i + 1] = v1;
		indices[i + 2] = v2;
		indices[i + 3] = v2;
		indices[i + 4] = v1;
		indices[i + 5] = v3;
	    }
	}
    }

    terrain::~terrain() { unload(); }


    void terrain::load()
    {
	geometry::properties properties{
	    .name = name,
	    .vertex_size = sizeof(terrain::vertex),
	    .vertex_count = (uint32_t)vertices.size(),
	    .vertices = vertices.data(),
	    .indices = indices,
	};
	geometry = geometry::geometry::create(properties);
	geometry->increment_generation();
    }

    void terrain::unload()
    {
	geometry->destroy();

	if (unique_id != invalid_32_id)
	{
	    identifier::release_id(unique_id);
	    unique_id = invalid_32_id;
	}
    }
}
