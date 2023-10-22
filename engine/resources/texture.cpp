#include "texture.h"

#include "renderer/renderer_frontend.h"

namespace egkr
{
	texture::shared_ptr texture::create(const renderer_frontend* context, const texture_properties& properties, const uint8_t* data)
	{
		auto tex = std::make_shared<texture>(context, properties);
		context->populate_texture(tex.get(), properties, data);

		return tex;
	}

	texture::texture(const renderer_frontend* renderer, const texture_properties& properties)
		: resource(properties.id, properties.generation, properties.name), renderer_{ renderer }, properties_{ properties }
	{
		data = (void*)properties.data;
	}

	texture::~texture() = default;

	void texture::destroy()
	{
		renderer_->free_texture(this);
	}

}