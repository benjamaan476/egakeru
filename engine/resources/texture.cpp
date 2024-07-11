#include "texture.h"

#include "renderer/renderer_types.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
	texture* texture::create()
	{
		return renderer->create_texture();
	}

	texture* texture::create(const properties& properties, const uint8_t* texture_data)
	{
		return renderer->create_texture(properties, texture_data);
	}
	void texture::create(const properties& properties, const uint8_t* texture_data, texture* out_texture)
	{
		return renderer->create_texture(properties, texture_data, out_texture);
	}

	texture::texture(const properties& properties)
		: resource(properties.id, properties.generation, properties.name), properties_{ properties }
	{
		data = (void*)properties.data;
	}

	texture::~texture()
	{
		if ((int)(properties_.flags & egkr::texture::flags::is_wrapped) == 0)
		{
			destroy();
		}
	}

	void texture::destroy()
	{
		if (properties_.data)
		{
			::free(properties_.data);
			properties_.data = nullptr;
		}
	}

	texture_map::shared_ptr texture_map::texture_map::create(const properties& properties)
	{
		{
			return renderer->create_texture_map(properties);
		}
	}

	texture_map::texture_map(const properties& properties)
		: minify{ properties.minify }, magnify{ properties.magnify }, repeat_u{ properties.repeat_u }, repeat_v{ properties.repeat_v }, repeat_w{ properties.repeat_w }, use{ properties.use }
	{

	}

	texture_map::texture_map::~texture_map()
	{
	}

	void texture_map::texture_map::free()
	{
		release();
	}
}
