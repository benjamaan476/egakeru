#pragma once

#include "keys.h"
#include "pch.h"

#include "resources/material.h"

#include <systems/system.h>

namespace egkr
{
    struct material_shader_uniform_location
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
    };

    struct ui_shader_uniform_location
    {
	uint32_t projection{};
	uint32_t view{};
	uint32_t diffuse_colour{};
	uint32_t diffuse_texture{};
	uint32_t model{};
    };

    class material_system : public system
    {
    public:
	using material_reference = uint32_t;
	using unique_ptr = std::unique_ptr<material_system>;
	static material_system* create();

	material_system();
	~material_system() override;

	bool init() override;
	bool shutdown() override;

	static material::shared_ptr get_default_material();
	static material::shared_ptr acquire(const std::string& name);
	static material::shared_ptr acquire(const material::properties& properties);
	static bool release(const material::shared_ptr& material);

	static void apply_global(uint32_t shader_id, const frame_data& frame_data, const float4x4& projection, const float4x4& view, const float4& ambient_colour, const float3& view_position, uint32_t mode);
	static void apply_instance(const material::shared_ptr& material, bool needs_update);
	static void apply_local(const material::shared_ptr& material, const float4x4& model);

	static bool set_irradiance(const texture::shared_ptr& irradiance_texture);
	[[nodiscard]] static const texture::shared_ptr& get_irradiance();

	static bool set_shadow_map(const texture::shared_ptr& texture, uint8_t index);
	[[nodiscard]] static const texture::shared_ptr& get_shadow_map();
	static void set_directional_light_space(const float4x4& light_space);
    private:
	static bool create_default_material();
    private:
	uint32_t max_material_count_{};

	material::shared_ptr default_material_;
	egkr::vector<material::shared_ptr> registered_materials_;
	std::unordered_map<std::string, material_reference> registered_materials_by_name_;

	material_shader_uniform_location material_locations_{};
	ui_shader_uniform_location ui_locations_{};

	uint32_t material_shader_id_{invalid_32_id};
	uint32_t ui_shader_id_{invalid_32_id};

	texture::shared_ptr irradiance_texture_;
	texture::shared_ptr shadow_texture_;
	float4x4 light_space_{1};
    };
}
