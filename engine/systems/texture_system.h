#pragma once

#include "pch.h"
#include "renderer/renderer_types.h"

#include "resources/texture.h"

#include <systems/system.h>

namespace egkr
{
    constexpr static std::string_view default_texture_name{"default"};
    constexpr static std::string_view default_diffuse_name{"default_diffuse_texture"};
    constexpr static std::string_view default_specular_name{"default_specular_texture"};
    constexpr static std::string_view default_normal_name{"default_normal_texture"};
    constexpr static std::string_view default_albedo_name{"default_albedo_texture"};
    constexpr static std::string_view default_metallic_name{"default_metallic_texture"};
    constexpr static std::string_view default_roughness_name{"default_roughness_texture"};
    constexpr static std::string_view default_ao_name{"default_ao_texture"};

    struct texture_system_configuration
    {
	uint32_t max_texture_count{};
    };

    struct load_parameters
    {
	char* name{};
	texture* out_texture{};
	texture* temp;
	uint32_t current_generation{invalid_32_id};

	resource::shared_ptr loaded_resource;
    };

    class texture_system : public system
    {
    public:
	using unique_ptr = std::unique_ptr<texture_system>;
	using texture_handle = uint32_t;

	static texture_system* create(const texture_system_configuration& properties);
	static texture::shared_ptr wrap_internal(
	    std::string_view name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency, bool is_writeable, bool register_texture, void* internal_data);

	explicit texture_system(const texture_system_configuration& properties);

	bool init() override;
	bool shutdown() override;

	static void resize(const texture::shared_ptr& texture, uint32_t width, uint32_t height, bool regenerate_internal_data);

	[[nodiscard]] static texture::shared_ptr acquire(const std::string& texture_name);
	[[nodiscard]] static texture::shared_ptr acquire_cube(const std::string& texture_name);
	[[nodiscard]] static texture::shared_ptr acquire_writable(const std::string& name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency);
	void release(std::string_view texture_name);

	static texture::shared_ptr get_default_texture();
	static texture::shared_ptr get_default_diffuse_texture();
	static texture::shared_ptr get_default_specular_texture();
	static texture::shared_ptr get_default_normal_texture();
	static texture::shared_ptr get_default_albedo_texture();
	static texture::shared_ptr get_default_metallic_texture();
	static texture::shared_ptr get_default_roughness_texture();
	static texture::shared_ptr get_default_ao_texture();
	static texture::shared_ptr get_default_ibl_texture();
    private:
	static texture::shared_ptr load_texture(const std::string& filepath, uint32_t id);
	static texture::shared_ptr load_cube_texture(const std::string& name, const egkr::vector<std::string>& texture_names, uint32_t id);

	static void load_job_success(void* params);
	static void load_job_fail(void* params);

	static bool job_start(void* params, void* result_data);
    private:
	texture::shared_ptr default_texture_;
	texture::shared_ptr default_diffuse_texture_;
	texture::shared_ptr default_specular_texture_;
	texture::shared_ptr default_normal_texture_;
	texture::shared_ptr default_albedo_texture_;
	texture::shared_ptr default_metallic_texture_;
	texture::shared_ptr default_roughness_texture_;
	texture::shared_ptr default_ao_texture_;
	texture::shared_ptr default_ibl_texture_;

	egkr::vector<texture::shared_ptr> registered_textures_;
	std::unordered_map<std::string, texture_handle> registered_textures_by_name_;

	uint32_t max_texture_count_{};
    };
}
