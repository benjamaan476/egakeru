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
	    egkr::vector<egkr::render_data> terrain;
	    egkr::vector<egkr::render_data> debug_geometries;
	    texture::shared_ptr irradiance_texture;
	    float4x4 directional_light_view;
	    float4x4 directional_light_projection;
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
	    uint32_t ambient_colour{};
	    uint32_t view_position{};
	    uint32_t diffuse_colour{};
	    uint32_t diffuse_texture{};
	    uint32_t specular_texture{};
	    uint32_t normal_texture{};
	    uint32_t shininess{};
	    uint32_t model{};
	    uint32_t mode{};
	    uint32_t directional_light{};
	    uint32_t point_light{};
	    uint32_t num_point_lights{};
	} terrain_shader_locations;

	struct pbr_shader_locations
	{
	    uint32_t projection{};
	    uint32_t view{};
	    uint32_t ambient_colour{};
	    uint32_t view_position{};
	    uint32_t albedo_texture{};
	    uint32_t normal_texture{};
	    uint32_t metallic_texture{};
	    uint32_t roughness_texture{};
	    uint32_t ao_texture{};
	    uint32_t ibl_texture{};
	    uint32_t shadow_texture{};
	    uint32_t light_space{};
	    uint32_t model{};
	    uint32_t directional_light{};
	    uint32_t point_lights{};
	    uint32_t num_point_lights{};
	    uint32_t properties{};
	    uint32_t mode{};
	} pbr_shader_locations;

	shader::shared_ptr material_shader;
	shader::shared_ptr debug_colour_shader;
	shader::shared_ptr terrain_shader;
	shader::shared_ptr pbr_shader;

	rendergraph::source* shadow_map_source;
	uint32_t frame_count;
	egkr::vector<texture_map::shared_ptr> shadow_maps;

	static scene* create();
	bool init() override;
	bool execute(const frame_data& frame_data) override;
	bool destroy() override;
	~scene() override = default;

	bool load_resources() override;
    private:
	static bool on_event(event::code code, void* /*sender*/, void* listener, const event::context& context);
    };
}
