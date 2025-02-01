#pragma once

#include "pch.h"
#include "resource.h"
#include "resources/geometry.h"
#include "transform.h"

namespace egkr
{
    class terrain : public resource, public transformable
    {
    public:
	struct configuration
	{
	    std::string name;
	    uint32_t tiles_x;
	    float scale_x;
	    uint32_t tiles_y;
	    float scale_y;
	};
	using shared_ptr = std::shared_ptr<terrain>;
	static shared_ptr create(const terrain::configuration& configuration);

	explicit terrain(const configuration& configuration);
	~terrain();

	struct vertex
	{
	    float3 position;
	    float3 normal;
	    float2 texture_coords;
	    float4 colour;
	    float4 tangent;
	    // std::array<float, 8> texture_weights;
	};

	void load();
	void unload();
	[[nodiscard]] const auto& get_geometry() const { return geometry; }
    private:
	uint32_t unique_id{invalid_32_id};
	std::string name;
	uint32_t tiles_x{};
	uint32_t tiles_y{};
	float scale_x{};
	float scale_y{};
	extent3d extents{};
	[[maybe_unused]] float3 origin{};

	//vertices and indices
	std::vector<vertex> vertices;
	std::vector<uint32_t> indices;

	geometry::shared_ptr geometry;
    };
}
