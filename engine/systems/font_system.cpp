#include "font_system.h"

namespace egkr
{
	static font_system::unique_ptr font_system_{};

	
	bool font_system::create(const renderer_frontend* renderer_context, const font_system_configuration& configuration)
	{
		font_system_ = std::make_unique<font_system>(renderer_context, configuration);
		return true;
	}

	font_system::font_system(const renderer_frontend* context, const font_system_configuration& configuration)
		:renderer_context_{ context }, configuration_{ configuration }
	{}

	bool font_system::init()
	{
		return false;
	}

	bool font_system::shutdown()
	{
		return false;
	}

	bool font_system::load_system_font(const system_font_configuration& configuration)
	{
		return false;
	}

}