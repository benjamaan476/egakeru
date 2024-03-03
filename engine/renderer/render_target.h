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
		enum class attachment_type
		{
			colour = 0x01,
			depth = 0x02,
			stencil = 0x04,
		};

		enum class attachment_source
		{
			default_source,
			view
		};

		enum class load_operation
		{
			dont_care,
			load
		};

		enum class store_operation
		{
			dont_care,
			store
		};

		struct attachment_configuration
		{
			attachment_type type;
			attachment_source source;
			load_operation load_operation;
			store_operation store_operation;
			bool present_after;
		};

		struct attachment
		{
			attachment_type type;
			attachment_source source;
			load_operation load_operation;
			store_operation store_operation;
			bool present_after;
			texture::texture* target;
		};

		class render_target
		{
		public:
			using shared_ptr = std::shared_ptr<render_target>;

			struct configuration
			{
				egkr::vector<attachment_configuration> attachments{};
			};

			render_target() = default;

			virtual ~render_target();
			virtual bool populate(egkr::vector<texture::texture*> attachments, renderpass::renderpass* renderpass, uint32_t width, uint32_t height) = 0;
			virtual bool free(bool free_internal_memory) = 0;

			void destroy()
			{
				attachments_.clear();
			}

		protected:
			egkr::vector<attachment> attachments_{};
		};
	}
}