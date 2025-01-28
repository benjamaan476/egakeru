#include "texture.h"

#include "renderer/renderer_types.h"
#include "renderer/renderer_frontend.h"

namespace egkr
{
    texture::shared_ptr texture::create() { return renderer->create_texture(); }

    texture::shared_ptr texture::create(const properties& texture_properties, const uint8_t* texture_data) { return renderer->create_texture(texture_properties, texture_data); }

    void texture::create(const properties& texture_properties, const uint8_t* texture_data, texture* out_texture) { renderer->create_texture(texture_properties, texture_data, out_texture); }

    texture::texture(const properties& texture_properties): resource(texture_properties.id, texture_properties.generation, texture_properties.name), properties_{texture_properties}
    {
	data = (void*)texture_properties.data;
    }

    texture::~texture()
    {
	if ((int)(properties_.texture_flags & egkr::texture::flags::is_wrapped) == 0)
	{
	    destroy();
	}
    }

    void texture::destroy()
    {
	if (properties_.data != nullptr)
	{
	    // ::free(properties_.data);
	    properties_.data = nullptr;
	}
    }

    texture_map::shared_ptr texture_map::texture_map::create(const properties& properties)
    {
	{
	    return renderer->create_texture_map(properties);
	}
    }

    texture_map::texture_map(const properties& map_properties)
        : minify{map_properties.minify}, magnify{map_properties.magnify}, repeat_u{map_properties.repeat_u}, repeat_v{map_properties.repeat_v}, repeat_w{map_properties.repeat_w}, use{map_properties.map_use}
    {
    }

    texture_map::texture_map::~texture_map() = default;

    void texture_map::texture_map::free() { release(); }
}
