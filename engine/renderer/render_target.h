#pragma once
#include "pch.h"

#include "resources/texture.h"

namespace egkr::render_target
{
	struct render_target
	{
		using shared_ptr = std::shared_ptr<render_target>;
		bool sync_to_window_size{};
		egkr::vector<texture::texture::shared_ptr> attachments{};

		void* internal_framebuffer;
};
}
