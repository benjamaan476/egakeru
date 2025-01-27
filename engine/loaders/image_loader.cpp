#include "image_loader.h"

#include "log/log.h"
#include "resources/texture.h"
#include "platform/filesystem.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include <stb_image.h>

namespace egkr
{
    image_loader::unique_ptr image_loader::create(const loader_properties& properties) { return std::make_unique<image_loader>(properties); }

    image_loader::image_loader(const loader_properties& properties): resource_loader{resource::type::image, properties} { }

    resource::shared_ptr image_loader::load(const std::string& name, void* params)
    {
	auto* parameters = std::bit_cast<image_resource_parameters*>(params);

	auto base_path = get_base_path();

	stbi_set_flip_vertically_on_load(parameters->flip_y);

	std::string filename;
	egkr::vector<std::string_view> extensions{".tga", ".png", ".jpg", ".bmp"};

	auto found{false};
	for (const auto& extension : extensions)
	{
	    filename = std::format("{}/{}{}", base_path, name, extension);

	    if (filesystem::does_path_exist(filename))
	    {
		found = true;
		break;
	    }
	}

	if (!found)
	{
	    LOG_ERROR("File not found: {}", filename);
	    return {};
	}

	auto file = filesystem::open(filename, file_mode::read, true);
	auto raw = filesystem::read_all(file);

	int32_t width{};
	int32_t height{};
	int32_t channels{};
	int32_t required_channels{4};

	auto *image_data = stbi_load_from_memory(raw.data(), (int32_t)raw.size(), &width, &height, &channels, required_channels);

	if (image_data != nullptr)
	{
	    [[maybe_unused]] static int count = 0;
	    count++;
	    LOG_TRACE("Loaded image: {}, {}", name, count);
	    resource::properties image_properties{};
	    image_properties.type = resource::type::image;
	    image_properties.name = name;
	    image_properties.full_path = filename;

	    image_properties.data = new (texture::properties);

	    auto* properties = (texture::properties*)image_properties.data;
	    properties->channel_count = (uint8_t)required_channels;
	    properties->width = (uint32_t)width;
	    properties->height = (uint32_t)height;
	    properties->generation = 0;
	    properties->id = 0;
	    properties->data = image_data;
	    properties->name = name;

	    for (auto y{0}; y < height; ++y)
	    {
		for (auto x{0}; x < width; x += 4)
		{
		    auto index = y * width + 4;

		    if (image_data[index + 3] < 255)
		    {
			properties->texture_flags |= texture::flags::has_transparency;
			break;
		    }
		}
	    }
	    return resource::create(image_properties);
	}
	else
	{
	    if (stbi_failure_reason())
	    {
		LOG_ERROR("Failed to load image {}, reason: {}", filename, stbi_failure_reason());
		stbi__err(nullptr, 0);
	    }
	    stbi_image_free(image_data);
	    return nullptr;
	}
    }

    bool egkr::image_loader::unload(const resource::shared_ptr& resource)
    {
	auto* data = (texture::properties*)resource->data;
	    [[maybe_unused]] static int count = 0;
	    count++;
	    LOG_TRACE("Unloaded image: {}, {}", data->name, count);
	stbi_image_free(data->data);
	delete data;
	data = nullptr;

	return true;
    }
}
