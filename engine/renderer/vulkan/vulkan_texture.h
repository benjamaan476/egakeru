#pragma once

#include "pch.h"
#include "resources/texture.h"
#include "image.h"

namespace egkr
{
	struct vulkan_context;
	class vulkan_texture : public texture
	{
	public:
		using shared_ptr = std::shared_ptr <vulkan_texture>;
		static shared_ptr create(const vulkan_context* context, const texture_properties& properties, const uint8_t* data);

		vulkan_texture(const vulkan_context* context, const texture_properties& properties, const uint8_t* data);
		~vulkan_texture();

		void destroy();
	private:
		const vulkan_context* context_{};
		image::shared_ptr image_{};
		vk::Sampler sampler_{};
	};
}