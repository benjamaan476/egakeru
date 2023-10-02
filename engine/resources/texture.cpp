#include "texture.h"

#include "renderer/vulkan/vulkan_texture.h"

namespace egkr
{
	texture::shared_ptr texture::create(const void* context, const texture_properties& properties, const uint8_t* data)
	{
		return vulkan_texture::create((vulkan_context*)context, properties, data);
	}

	texture::texture(const texture_properties& /*properties*/)
	{

	}
}