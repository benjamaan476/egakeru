#pragma once
#include "pch.h"

#include "resources/texture.h"

namespace egkr
{
	namespace renderpass
	{
		class renderpass;
	}
	namespace render_target
	{
		struct render_target
		{
			using shared_ptr = std::shared_ptr<render_target>;

			explicit render_target(const renderer_backend* backend) : backend{ backend }
			{
			}
			virtual ~render_target()
			{
				destroy();
			}

			void destroy()
			{
				attachments.clear();
			}

			const renderer_backend* backend{};
			bool sync_to_window_size{};
			egkr::vector<texture::texture::shared_ptr> attachments{};

			virtual bool populate(egkr::vector<texture::texture::shared_ptr> attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height) = 0;
			virtual bool free(bool free_internal_memory) = 0;
		};
	}
}