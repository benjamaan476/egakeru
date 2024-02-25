#include "render_view.h"
#include "renderer/views/render_view_ui.h"
#include "renderer/views/render_view_world.h"
#include "renderer/views/render_view_skybox.h"

#include <renderer/renderer_frontend.h>
#include <systems/camera_system.h>

namespace egkr::render_view
{
	render_view::shared_ptr render_view::create(const configuration& configuration)
	{
		if (configuration.type == type::world)
		{
			return std::make_shared<render_view_world>(configuration);
		}
		else if (configuration.type == type::ui)
		{
			return std::make_shared<render_view_ui>(configuration);
		}
		else if (configuration.type == type::skybox)
		{
			return std::make_shared<render_view_skybox>(configuration);
		}
		else
		{
			LOG_ERROR("Unrecognized view type");
			return nullptr;
		}
		//return backend->create_render_view(configuration);
	}

	render_view::render_view(const configuration& configuration)
		: name_{ configuration.name }, 
		width_{ configuration.width },
		height_{ configuration.height },
		type_{ configuration.type },
		custom_shader_name_{ configuration.custom_shader_name }
	{
		camera_ = camera_system::get_default();
		for (const auto& pass : configuration.passes)
		{
			renderpasses_.push_back(renderer->get_renderpass(pass.name));
		}
	}

	bool render_view::on_event(event_code code, void* /*sender*/, void* listener, const event_context& context)
	{
		render_view* view = (render_view*)listener;

		switch (code)
		{
		case event_code::render_mode:
		{
			context.get(0, view->mode_);
		} break;
		default:
			return false;
		}
		return true;
	}
}