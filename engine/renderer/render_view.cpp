#include "render_view.h"

namespace egkr::render_view
{
	render_view::shared_ptr render_view::create(const renderer_backend* backend, const configuration& configuration)
	{
		return backend->create_render_view(configuration);
	}

	render_view::render_view(const configuration& configuration)
		: 
	{
	}
}