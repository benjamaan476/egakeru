#pragma once
#include "pch.h"
#include "renderer/render_graph.h"
#include "renderer/render_target.h"

namespace egkr::pass
{
    struct shadow : public egkr::rendergraph::pass
    {
    public:
	struct packet_data
	{
	    egkr::vector<egkr::render_data> geometries;
	    egkr::vector<egkr::render_data> terrain;
	} data;

	struct properties
	{
	    uint16_t resolution;
	};

	struct shadow_shader_locations
	{
	    uint32_t projection{};
	    uint32_t view{};
	    uint32_t model{};
	    uint32_t light_space{};
	    uint32_t colour_map{};
	} shadow_shader_locations;

	static shadow* create();
	explicit shadow();

	bool init() override;
	bool execute(const frame_data& frame_data) override;
	bool load_resources() override;
	texture::shared_ptr get_texture(render_target::attachment_type type, uint8_t index) const override;
	bool destroy() override;
    private:
	shader::shared_ptr shadow_shader;
	uint16_t resolution;
	uint32_t instance_count;
	texture_map::shared_ptr default_colour_map;
	egkr::vector<texture::shared_ptr> colour_textures;
	egkr::vector<texture::shared_ptr> depth_textures;
    };
}
