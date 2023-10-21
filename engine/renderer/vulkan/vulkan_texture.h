#pragma once

#include "pch.h"
#include "resources/texture.h"
#include "image.h"

namespace egkr
{
	struct vulkan_texture_state
	{
		const vulkan_context* context{};
		image::shared_ptr image;
	};
}