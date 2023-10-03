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
		static unique_ptr create(const texture_system_properties& properties);

		explicit texture_system(const texture_system_properties& properties);
		bool init();
		void shutdown();

		texture::shared_ptr acquire(std::string_view texture_name);
		void release(std::string_view texture_name);

		texture::shared_ptr get_default_texture();
	private:
		texture::shared_ptr default_texture_{};

		egkr::vector<texture::shared_ptr> registered_textures_{};
		std::unordered_map<std::string, uint32_t> registered_textures_by_name_{};

		uint32_t max_texture_count_{};
	};
}