#include "texture_system.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <filesystem>



namespace egkr
{
	texture_system::unique_ptr texture_system::create(const texture_system_properties& properties)
	{
		return std::make_unique<texture_system>(properties);
	}

	texture_system::texture_system(const texture_system_properties& properties)
		: max_texture_count_{ properties.max_texture_count }
	{
	}

	bool texture_system::init()
	{
		return false;
	}
	void texture_system::shutdown()
	{
	}
	texture::shared_ptr texture_system::acquire(std::string_view texture_name)
	{
		return texture::shared_ptr();
	}
	void texture_system::release(std::string_view texture_name)
	{
	}
	texture::shared_ptr texture_system::get_default_texture()
	{
		return texture::shared_ptr();
	}
}