#include "renderpass.h"
#include "renderer_types.h"

namespace egkr::renderpass
{

	renderpass::renderpass(const renderer_backend* renderer, const configuration& configuration) :  render_area_{configuration.render_area}, renderer_{ renderer }
	{
		clear_colour_ = configuration.clear_colour;
		clear_flags_ = configuration.clear_flags;
	}
	
	renderpass::~renderpass()
	{
		render_targets.clear();
	}

void renderpass::set_render_area(uint32_t width, uint32_t height)
{
	if (width != render_area_.z || height != render_area_.w)
	{
		render_area_ = { 0, 0, width, height };
	}
}
}