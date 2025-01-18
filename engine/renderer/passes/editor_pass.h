#pragma once
#include "pch.h"

#include "resources/shader.h"
#include "renderer/renderpass.h"
#include "renderer/render_graph.h"

#include <editor_gizmo.h>
#include "event.h"

namespace egkr::pass
{
    class editor : public egkr::rendergraph::pass
    {
    public:
	struct packet_data
	{
	    float4 ambient_colour{};
	    int32_t render_mode{};
	    egkr::editor::gizmo gizmo;
	} data;

	struct debug_colour_shader_locations
	{
	    uint32_t projection{};
	    uint32_t view{};
	    uint32_t model{};
	} debug_shader_locations;

	shader::shared_ptr material_shader;
	shader::shared_ptr colour_shader;

	static editor* create();
	bool init() override;
	bool execute(const frame_data& frame_data) const override;
	bool destroy() override;
	 ~editor() override = default;
    private:
	static bool on_event(event::code code, void* /*sender*/, void* listener, const event::context& context);
    };
}
