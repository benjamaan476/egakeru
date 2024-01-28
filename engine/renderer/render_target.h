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
		class render_target
		{
		public:
			using shared_ptr = std::shared_ptr<render_target>;

			render_target() {}

			virtual ~render_target();
			virtual bool populate(egkr::vector<texture::texture*> attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height) = 0;
			virtual bool free(bool free_internal_memory) = 0;

			void destroy()
			{
				attachments_.clear();
			}

		protected:
			bool sync_to_window_size_{};
			egkr::vector<texture::texture*> attachments_{};
		};
	}
}