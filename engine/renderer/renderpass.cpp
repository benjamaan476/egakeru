#include "renderpass.h"
#include <renderer/renderer_frontend.h>

namespace egkr::renderpass
{
	renderpass::shared_ptr renderpass::create(const configuration& configuration)
	{
		return renderer->create_renderpass(configuration);
	}

	renderpass::renderpass(const configuration& configuration)
		: clear_colour_{ configuration.clear_colour }, clear_flags_{ configuration.pass_clear_flags }, depth_{ configuration.depth }, stencil_{ configuration.stencil }
	{
		render_targets_[0] = render_target::render_target::create(configuration.target.attachments);
		render_targets_[1] = render_target::render_target::create(configuration.target.attachments);
		render_targets_[2] = render_target::render_target::create(configuration.target.attachments);
	}

	renderpass::~renderpass()
	{
		render_targets_.clear();
	}
}
