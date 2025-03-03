#pragma once
#include "pch.h"
#include "resources/geometry.h"
#include "resources/shader.h"
#include "renderer/render_graph.h"

namespace egkr::pass
{
    class oit : public egkr::rendergraph::pass
    {
	public:
	    struct packet_data
	    {
		egkr::vector<egkr::render_data> oit_geometries; 
	    } data;

	    struct oit_shader_locations
	    {
		uint32_t projection{};
		uint32_t view{};
		uint32_t model{};
	    } oit_shader_locations;

	    shader::shared_ptr oit_colour_shader;
	    shader::shared_ptr oit_weighted_shader;

	    static oit* create();
	    bool init() override;
	    bool execute(const frame_data& frame_data) const override;
	    bool destroy() override;

	    ~oit() override = default;
    };
}
