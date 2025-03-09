#pragma once
#include "pch.h"


#include "resource.h"
#include "texture.h"
#include <unordered_map>

namespace egkr
{
    constexpr static std::string_view default_material_name_{"default"};


    class material : public resource
    {
    public:
	enum class type : uint8_t
	{
	    phong,
	    pbr
	};

	struct properties
	{
	    std::string name{"default"};
	    std::string shader_name{"Shader.Material"};
	    float shininess{};
	    float4 diffuse_colour{0.588F, 0.588F, 0.588F, 1.F};
	    type material_type{type::phong};

	    // ["albedo", { "roof_albedo", {linear, linear, repeat...}}]
	    std::unordered_map<std::string, std::pair<std::string, texture_map::properties>> texture_maps;
	};

	using shared_ptr = std::shared_ptr<material>;
	static shared_ptr create(const properties& properties);

	explicit material(const properties& properties);
	~material();

	void free();

	void set_diffuse_colour(const float4 diffuse);
	[[nodiscard]] const auto& get_diffuse_colour() const { return diffuse_colour_; }

	void set_diffuse_map(const texture_map::shared_ptr& map) { diffuse_map_ = map; }
	[[nodiscard]] const auto& get_diffuse_map() const { return diffuse_map_; }
	[[nodiscard]] auto& get_diffuse_map() { return diffuse_map_; }

	void set_specular_map(const texture_map::shared_ptr& map) { specular_map_ = map; }
	[[nodiscard]] const auto& get_specular_map() const { return specular_map_; }
	[[nodiscard]] auto& get_specular_map() { return specular_map_; }

	void set_normal_map(const texture_map::shared_ptr& map) { normal_map_ = map; }
	[[nodiscard]] const auto& get_normal_map() const { return normal_map_; }
	[[nodiscard]] auto& get_normal_map() { return normal_map_; }

	void set_albedo_map(const texture_map::texture_map::shared_ptr& map) { albedo_map_ = map; }
	[[nodiscard]] const auto& get_albedo_map() const { return albedo_map_; }
	[[nodiscard]] auto& get_albedo_map() { return albedo_map_; }

	void set_metallic_map(const texture_map::texture_map::shared_ptr& map) { metallic_map_ = map; }
	[[nodiscard]] const auto& get_metallic_map() const { return metallic_map_; }
	[[nodiscard]] auto& get_metallic_map() { return metallic_map_; }

	void set_roughness_map(const texture_map::texture_map::shared_ptr& map) { roughness_map_ = map; }
	[[nodiscard]] const auto& get_roughness_map() const { return roughness_map_; }
	[[nodiscard]] auto& get_roughness_map() { return roughness_map_; }

	void set_ao_map(const texture_map::texture_map::shared_ptr& map) { ao_map_ = map; }
	[[nodiscard]] const auto& get_ao_map() const { return ao_map_; }
	[[nodiscard]] auto& get_ao_map() { return ao_map_; }

	void set_ibl_map(const texture_map::texture_map::shared_ptr& map) { ibl_map_ = map; }
	[[nodiscard]] const auto& get_ibl_map() const { return ibl_map_; }
	[[nodiscard]] auto& get_ibl_map() { return ibl_map_; }


	void set_shininess(float shininess) { shininess_ = shininess; }
	[[nodiscard]] const auto& get_shininess() const { return shininess_; }

	[[nodiscard]] const auto& get_shader_id() const { return shader_id_; }
	[[nodiscard]] const auto& get_shader_name() const { return shader_name_; }

	[[nodiscard]] const auto& get_internal_id() const { return internal_id_; }
	void set_internal_id(uint32_t id) { internal_id_ = id; }

	[[nodiscard]] auto get_render_frame() const { return render_frame_number_; }
	void set_render_frame(uint64_t frame) { render_frame_number_ = frame; }

	[[nodiscard]] auto get_draw_index() const { return draw_index_; }
	void set_draw_index(uint64_t frame) { draw_index_ = frame; }

	[[nodiscard]] auto get_material_type() const { return material_type; }
    private:
	float shininess_{32.F};
	float4 diffuse_colour_{1.F};
	texture_map::shared_ptr diffuse_map_;
	texture_map::shared_ptr specular_map_;
	texture_map::shared_ptr normal_map_;
	texture_map::shared_ptr albedo_map_;
	texture_map::shared_ptr metallic_map_;
	texture_map::shared_ptr roughness_map_;
	texture_map::shared_ptr ao_map_;
	texture_map::shared_ptr ibl_map_;
	material::type material_type{type::phong};

	std::string shader_name_;
	uint32_t shader_id_{invalid_32_id};
	uint32_t internal_id_{invalid_32_id};
	uint64_t render_frame_number_{invalid_64u_id};
	uint64_t draw_index_{invalid_64u_id};
    };
}
