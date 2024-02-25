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

	struct texture_system_configuration
	{
		uint32_t max_texture_count{};
	};

	struct load_parameters
	{
		char* name{};
		texture::texture* out_texture{};
		texture::texture* temp;
		uint32_t current_generation{invalid_32_id};

		resource::shared_ptr resource;
	};

	class texture_system : public system
	{
	public:
		using unique_ptr = std::unique_ptr<texture_system>;
		using texture_handle = uint32_t;

		static texture_system* create(const texture_system_configuration& properties);
		static texture::texture* wrap_internal(std::string_view name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency, bool is_writeable, bool register_texture, void* internal_data);

		explicit texture_system(const texture_system_configuration& properties);

		bool init() override;
		bool update(float delta_time) override;
		bool shutdown() override;

		static void resize(texture::texture* texture, uint32_t width, uint32_t height, bool regenerate_internal_data);

		static texture::texture* acquire(std::string_view texture_name);
		static texture::texture* acquire_cube(std::string_view texture_name);
		static texture::texture* acquire_writable(std::string_view name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency);
		void release(std::string_view texture_name);

		static texture::texture* get_default_texture();
		static texture::texture* get_default_diffuse_texture();
		static texture::texture* get_default_specular_texture();
		static texture::texture* get_default_normal_texture();
	private:
		static texture::texture* load_texture(const std::string& filepath, uint32_t id);
		static texture::texture* load_cube_texture(std::string_view name, const egkr::vector<std::string>& texture_names, uint32_t id);

		static void load_job_success(void* params);
		static void load_job_fail(void* params);

		static bool job_start(void* params, void* result_data);

	private:
		texture::texture* default_texture_{};
		texture::texture* default_diffuse_texture_{};
		texture::texture* default_specular_texture_{};
		texture::texture* default_normal_texture_{};

		egkr::vector<texture::texture*> registered_textures_{};
		std::unordered_map<std::string, texture_handle> registered_textures_by_name_{};

		uint32_t max_texture_count_{};
	};
}