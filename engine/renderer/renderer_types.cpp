#include "renderer_types.h"
#include "plugins/renderer/vulkan/renderer_vulkan.h"

namespace egkr
{
	renderer_backend::unique_ptr renderer_backend::create(const platform::shared_ptr& platform)
	{
		return renderer_vulkan::create(platform);
	}
}
