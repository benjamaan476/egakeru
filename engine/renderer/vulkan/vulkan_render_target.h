#include "renderer/render_target.h"
#include "renderer/renderpass.h"

#include <vulkan/vulkan.hpp>

namespace egkr
{
	struct vulkan_context;
	namespace render_target
	{
		class vulkan_render_target : public render_target
		{
		public:
			static shared_ptr create(const vulkan_context* context);

			vulkan_render_target(const vulkan_context* context)
				: render_target(), context_{ context }
			{ }

			~vulkan_render_target() override;

			bool populate(egkr::vector<texture::texture::shared_ptr> attachment, renderpass::renderpass* renderpass, uint32_t width, uint32_t height) override;
			bool free(bool free_internal_memory) override;

			const auto& get_framebuffer() const { return framebuffer_; }

		private:
			const vulkan_context* context_{};
			vk::Framebuffer framebuffer_{};
		};
	}
}