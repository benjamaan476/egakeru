#pragma once
#include "pch.h"
#include "resources/geometry.h"
#include "resources/shader.h"
#include "renderer/render_graph.h"
#include "resources/ui_text.h"
#include "resources/mesh.h"

namespace egkr::pass
{
    class ui : public egkr::rendergraph::pass
    {
    public:
	struct packet_data
	{
	    egkr::vector<egkr::render_data> ui_geometries;
	    egkr::vector<egkr::mesh::weak_ptr> mesh_data;
	    egkr::vector<std::weak_ptr<egkr::text::ui_text>> texts;
	} data;

	struct shader_locations
	{
	    uint32_t diffuse_map_location{};
	    uint32_t diffuse_colour_location{};
	    uint32_t model_location{};
	} shader_locations;

	shader::shared_ptr shader;

	static ui* create();

	bool init() override;
	bool execute(const frame_data& frame_data) override;
	bool destroy() override;

	~ui() override = default;
    };
}
