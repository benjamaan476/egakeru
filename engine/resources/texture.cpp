#include "texture.h"

#include "renderer/renderer_types.h"

namespace egkr::texture
{
	texture::shared_ptr texture::create(const renderer_backend* context, const properties& properties, const uint8_t* data)
	{
		auto tex = context->create_texture(properties, data);

		return tex;
	}

	texture::texture(const renderer_backend* renderer, const properties& properties)
		: resource(properties.id, properties.generation, properties.name), properties_{ properties }, renderer_{ renderer }
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
		if (renderer_)
		{
			renderer_ = nullptr;
		}
	}

}