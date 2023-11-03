#include "renderpass.h"
#include "renderer_types.h"

namespace egkr::renderpass
{

	renderpass::renderpass(const renderer_backend* renderer, const configuration& configuration) :  render_area_{configuration.render_area}, renderer_{ renderer }
	{
		clear_colour_ = configuration.clear_colour;
		clear_flags_ = configuration.clear_flags;
	}
	
	renderpass::~renderpass() = default;
}