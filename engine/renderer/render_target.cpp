#include "render_target.h"
#include "engine/engine.h"
#include <application/application.h>

namespace egkr::render_target
{
    render_target::shared_ptr render_target::create(const egkr::vector<attachment>& attachments, renderpass::renderpass* pass, uint32_t width, uint32_t height)
    {
	return engine::get()->get_renderer()->create_render_target(attachments, pass, width, height);
    }

    render_target::shared_ptr render_target::create(const egkr::vector<attachment_configuration>& attachments) { return engine::get()->get_renderer()->create_render_target(attachments); }

    render_target::render_target() { }

    render_target::~render_target() { destroy(); }

    void render_target::destroy()
    {
	for (auto& attachment : attachments_)
	{
	    attachment.texture_attachment.reset();
	}
	attachments_.clear();
    }
}
