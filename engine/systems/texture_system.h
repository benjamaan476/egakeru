#pragma once

#include "pch.h"
#include "renderer/renderer_types.h"

#include "resources/texture.h"
namespace egkr
{
	constexpr static std::string_view default_texture_name{"default"};

	struct texture_system_properties
	{
		uint32_t max_texture_count{};
	};

	class texture_system
	{
	public:
		using unique_ptr = std::unique_ptr<texture_system>;
		using texture_handle = uint32_t;

		static void create(const void* renderer_context, const texture_system_properties& properties);

		texture_system(const void* renderer_context, const texture_system_properties& properties);
		bool init();
		static void shutdown();

		static texture::shared_ptr acquire(std::string_view texture_name);
		void release(std::string_view texture_name);

		static texture::shared_ptr get_default_texture();
	private:
		static bool load_texture(std::string_view filepath, texture::shared_ptr& texture);


	private:
		const void* renderer_context_{};
		texture::shared_ptr default_texture_{};

		egkr::vector<texture::shared_ptr> registered_textures_{};
		std::unordered_map<std::string, texture_handle> registered_textures_by_name_{};

		uint32_t max_texture_count_{};
	};
}