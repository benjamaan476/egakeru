#pragma once

#include "pch.h"
#include "renderer/renderer_frontend.h"
#include "renderer/renderer_types.h"

#include "resources/texture.h"
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

	class texture_system
	{
	public:
		using unique_ptr = std::unique_ptr<texture_system>;
		using texture_handle = uint32_t;

		static void create(const renderer_frontend* renderer_context, const texture_system_configuration& properties);
		static texture::shared_ptr wrap_internal(std::string_view name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency, bool is_writeable, bool register_texture, void* internal_data);

		texture_system(const renderer_frontend* renderer_context, const texture_system_configuration& properties);
		static bool init();
		static void shutdown();

		static void resize(texture* texture, uint32_t width, uint32_t height, bool regenerate_internal_data);

		static texture::shared_ptr acquire(std::string_view texture_name);
		static texture::shared_ptr acquire_writable(std::string_view name, uint32_t width, uint32_t height, uint8_t channel_count, bool has_transparency);
		void release(std::string_view texture_name);

		static texture::shared_ptr get_default_texture();
		static texture::shared_ptr get_default_diffuse_texture();
		static texture::shared_ptr get_default_specular_texture();
		static texture::shared_ptr get_default_normal_texture();
	private:
		static texture::shared_ptr load_texture(std::string_view filepath, uint32_t id);


	private:
		const renderer_frontend* renderer_context_{};
		texture::shared_ptr default_texture_{};
		texture::shared_ptr default_diffuse_texture_{};
		texture::shared_ptr default_specular_texture_{};
		texture::shared_ptr default_normal_texture_{};

		egkr::vector<texture::shared_ptr> registered_textures_{};
		std::unordered_map<std::string, texture_handle> registered_textures_by_name_{};

		uint32_t max_texture_count_{};
	};
}