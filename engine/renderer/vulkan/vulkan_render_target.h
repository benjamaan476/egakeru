#include "renderer/render_target.h"
#include "renderer/renderpass.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct vulkan_context;
	namespace render_target
	{
		struct vulkan_render_target : public render_target
		{
			vk::Framebuffer framebuffer_{};
			static shared_ptr create(const renderer_backend* backend, const vulkan_context* context);

			vulkan_render_target(const renderer_backend* backend, const vulkan_context* context)
				: render_target(backend), context_{ context }
			{ }

			~vulkan_render_target() override;

			bool populate(egkr::vector<texture::texture::shared_ptr> attachment, renderpass::renderpass* renderpass, uint32_t width, uint32_t height) override;

			bool free(bool free_internal_memory) override;

			const vulkan_context* context_{};
		};
	}
}