#include "render_target.h"

namespace egkr::render_target
{
	render_target::render_target(const configuration& configuration)
	:	attachments_{configuration.attachments}
	{

	}

	render_target::~render_target()
	{
		destroy();
	}
}