#pragma once

#include <systems/font_system.h>
#include <renderer/render_view.h>

namespace egkr
{
	struct engine_configuration
	{
		uint32_t width{ 800 };
		uint32_t height{ 600 };
		std::string name{};

		font_system::configuration font_systen_configuration_{};
		render_view::configuration render_view_configuration_{};
	};
}