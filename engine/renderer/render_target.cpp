#include "render_target.h"
#include <renderer/renderer_frontend.h>

namespace egkr::render_target
{
	render_target::shared_ptr render_target::create(const egkr::vector<attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height)
	{
		return renderer->create_render_target(attachments, pass, width, height);
	}

	render_target::shared_ptr render_target::create(const egkr::vector<attachment_configuration>& attachments)
	{
		return renderer->create_render_target(attachments);
	}


	render_target::render_target()
	{

	}

	render_target::~render_target()
	{
		destroy();
	}
}
