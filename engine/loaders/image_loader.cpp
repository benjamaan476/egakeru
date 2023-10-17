#include "image_loader.h"

#include "resources/texture.h"
#include "platform/filesystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace egkr
{
	image_loader::unique_ptr image_loader::create(const loader_properties& properties)
	{
		return std::make_unique<image_loader>(properties);
	}

	image_loader::image_loader(const loader_properties& properties)
		: resource_loader{resource_type::image, properties}
	{

	}

	resource::shared_ptr image_loader::load(std::string_view name)
	{
		auto base_path = get_base_path();

		// Base path/{name}{extension}
		constexpr std::string_view format_string{ "%s/%s%s" };
		stbi_set_flip_vertically_on_load(true);

		char buff[128];
		egkr::vector<std::string_view> extensions{".tga", ".png", ".jpg", ".bmp"};

		auto found{ false };
		for (const auto& extension : extensions)
		{
			sprintf_s(buff, format_string.data(), base_path.data(), name.data(), extension.data());

			if (filesystem::does_path_exist(buff))
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			LOG_ERROR("File not found: {}", buff);
			return {};
		}

		int32_t width{};
		int32_t height{};
		int32_t channels{};
		int32_t required_channels{ 4 };

		auto image_data = (uint8_t*)stbi_load(buff, &width, &height, &channels, required_channels);

		if (image_data)
		{
			texture_properties properties{};
			properties.channel_count = required_channels;
			properties.width = width;
			properties.height = height;
			properties.generation = 0;
			properties.id = 0;
			properties.data = image_data;

			for (auto y{ 0 }; y < height; ++y)
			{
				for (auto x{ 0 }; x < width; x += 4)
				{
					auto index = y * width + 4;

					if (image_data[index + 3] < 255)
					{
						properties.has_transparency = true;
						break;
					}
				}
			}

			resource_properties image_properties{};
			image_properties.type = resource_type::image;
			image_properties.name = name;
			image_properties.full_path = buff;

			// Deleted by unload
			image_properties.data = new(texture_properties);
			*(texture_properties*)(image_properties.data) = properties;

			return std::make_shared<resource>(image_properties);
			//return resource::create(image_properties);
		}
		else
		{
			if (stbi_failure_reason())
			{
				LOG_ERROR("Failed to load image {}, reason: {}", buff, stbi_failure_reason());
				stbi__err(0, 0);
			}
			return nullptr;
		}
	}

	bool egkr::image_loader::unload(const resource::shared_ptr& resource)
	{
		auto* data = (texture_properties*)resource->data;
		delete data;

		return true;
	}
}