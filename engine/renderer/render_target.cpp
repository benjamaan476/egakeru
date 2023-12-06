#include "render_target.h"

namespace egkr::render_target
{
	render_target::~render_target()
	{
		destroy();
	}
}