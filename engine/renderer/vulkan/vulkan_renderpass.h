#pragma once

#include "pch.h"
#include "command_buffer.h"
#include "renderer/renderer_types.h"
#include "renderer/renderpass.h"

namespace egkr
{
	struct vulkan_context;

	namespace renderpass
	{
		class vulkan_renderpass : public renderpass::renderpass, std::enable_shared_from_this<vulkan_renderpass>
		{
		public:
			using shared_ptr = std::shared_ptr<vulkan_renderpass>;
			static shared_ptr create(const renderer_backend* renderer, vulkan_context* context, const configuration& configuration);

			vulkan_renderpass(const renderer_backend* renderer, vulkan_context* context, const configuration& configuration);
			~vulkan_renderpass() override;

			void destroy();

			void begin(command_buffer& command_buffer, vk::Framebuffer framebuffer);
			static void end(command_buffer& command_buffer);

			void set_extent(uint4 extent);

			const auto& get_handle()
			{
				return renderpass_;
			}

			bool populate(float depth, float stencil, bool has_previous, bool has_next) override;
			bool begin(render_target::render_target* render_target) const override;
			bool end() override;
			void free() override;

		private:
			vulkan_context* context_{};
			vk::RenderPass renderpass_{};
			float_t depth_{};
			uint32_t stencil_{};

			state state_{};
		};
	}
}
