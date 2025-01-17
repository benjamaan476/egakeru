#pragma once
#include "pch.h"
#include "resources/shader.h"
#include "renderer/renderpass.h"
#include "renderer/render_graph.h"

namespace egkr::pass
{
	struct skybox : public egkr::rendergraph::pass
	{
	public:
		struct shader_locations
		{
			uint32_t projection{};
			uint32_t view{};
			uint32_t cubemap{};
		} shader_locations;
		shader::shared_ptr shader;

		bool init() override;
		bool execute(const frame_data& frame_data) const override;
		bool destroy() override;

	private:
		renderpass::renderpass::shared_ptr pass;
	};
}
