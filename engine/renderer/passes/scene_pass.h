#pragma once
#include "pch.h"
#include "resources/geometry.h"
#include "resources/shader.h"
#include "renderer/render_graph.h"
#include "event.h"

namespace egkr::pass
{
    struct scene : public egkr::rendergraph::pass
    {
    public:
	struct packet_data
	{
	    uint32_t render_mode;
	    float4 ambient_colour;
	    egkr::vector<egkr::render_data> geometries;
	    egkr::vector<egkr::terrain_render_data> terrain;
	    egkr::vector<egkr::render_data> debug_geometries;
	} data;

	struct debug_colour_shader_locations
	{
	    uint32_t projection{};
	    uint32_t view{};
	    uint32_t model{};
	} debug_shader_locations;

	struct terrain_shader_locations
	{
	    uint32_t projection{};
	    uint32_t view{};
	    uint32_t model{};
	    uint32_t ambient_colour{};
	    uint32_t view_position{};
	    uint32_t material_location{};
	    uint32_t mode{};
	    std::array<uint32_t, 12> samplers;
	    uint32_t directional_light{};
	    uint32_t point_light{};
	    uint32_t num_point_lights{};
	    uint32_t num_materials{};
	} terrain_shader_locations;

	shader::shared_ptr material_shader;
	shader::shared_ptr debug_colour_shader;
	shader::shared_ptr terrain_shader;

	static scene* create();
	bool init() override;
	bool execute(const frame_data& frame_data) const override;
	bool destroy() override;
	~scene() override = default;
    private:
	static bool on_event(event::code code, void* /*sender*/, void* listener, const event::context& context);
    };
}
