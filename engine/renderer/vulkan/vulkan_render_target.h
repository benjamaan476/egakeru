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
			static shared_ptr create(const vulkan_context* context, const egkr::vector<egkr::render_target::attachment>& attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height);
			static shared_ptr create(const vulkan_context* context, const egkr::vector<attachment_configuration>& attachments);

			vulkan_render_target(const vulkan_context* context, const egkr::vector<attachment_configuration>& attachments);
			vulkan_render_target(const vulkan_context* context, const egkr::vector<egkr::render_target::attachment>& attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height);

			~vulkan_render_target() override;

			bool free(bool free_internal_memory) override;

			const auto& get_framebuffer() const { return framebuffer_; }

		private:
			const vulkan_context* context_{};
			vk::Framebuffer framebuffer_{};
		};
	}
}
