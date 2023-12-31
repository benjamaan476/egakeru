#include "texture.h"

#include "renderer/renderer_types.h"

namespace egkr
{
	namespace texture
	{
		texture::shared_ptr texture::create(const renderer_backend* context, const properties& properties, const uint8_t* data)
		{
			return context->create_texture(properties, data);
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

	}

	namespace texture_map
	{
		texture_map::shared_ptr texture_map::texture_map::create(const renderer_backend* context, const properties& properties)
		{
			{
				return context->create_texture_map(properties);
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
}