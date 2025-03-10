#pragma once
#include "pch.h"
#include "resources/shader.h"
#include "renderer/render_graph.h"
#include "resources/skybox.h"

namespace egkr::pass
{
    struct skybox : public egkr::rendergraph::pass
    {
    public:
	struct packet_data
	{
	    egkr::skybox::skybox::shared_ptr skybox_data;
	} data;

	struct shader_locations
	{
	    uint32_t projection{};
	    uint32_t view{};
	    uint32_t cubemap{};
	} shader_locations;
	shader::shared_ptr shader;

	static skybox* create();

	bool init() override;
	bool execute(const frame_data& frame_data) override;
	bool destroy() override;
	~skybox() override = default;
    };
}
