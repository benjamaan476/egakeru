#include "texture.h"

#include "renderer/renderer_frontend.h"

namespace egkr
{
	texture::shared_ptr texture::create(const renderer_frontend* context, const texture_properties& properties, const uint8_t* data)
	{
		auto tex = std::make_shared<texture>(properties);
		context->populate_texture(tex.get(), properties, data);

		return tex;
	}

	texture::texture(const texture_properties& properties)
		: resource{properties.id, properties.generation}
	{
	}

	texture::~texture() = default;

	void texture::destroy(const renderer_frontend* renderer)
	{
		renderer->free_texture(this);
	}

}