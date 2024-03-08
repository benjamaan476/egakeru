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
			auto renderpass = renderpass::renderpass::create(pass);
			//for (auto& target : renderpass->get_render_targets())
			{
				//target = render_target::render_target::create(pass.target.attachments);
			}
			renderpasses_.push_back(renderpass);
		}

		regenerate_render_targets();
	}

	void render_view::regenerate_render_targets()
	{
		for (const auto& pass : renderpasses_)
		{
			for (auto i{ 0U }; i < pass->get_render_target_count(); ++i)
			{
				auto& target = pass->get_render_target(i);
				if (target)
				{
					target->free(false);
				}
				for (auto a{ 0U }; a < target->get_attachments().size(); ++a)
				{
					auto& attachment = target->get_attachments()[a];
					if (attachment.source == render_target::attachment_source::default_source)
					{
						if (attachment.type == render_target::attachment_type::colour)
						{
							attachment.texture = renderer->get_backend()->get_window_attachment(i);
						}
						else if (attachment.type == render_target::attachment_type::depth)
						{
							attachment.texture = renderer->get_backend()->get_depth_attachment(i);
						}
						else
						{
							LOG_FATAL("Unsupported attachment type");
							continue;
						}
					}
					else if (attachment.source == render_target::attachment_source::view)
					{
						regenerate_attachment_target(a, attachment);
					}
				}

				target = render_target::render_target::create(target->get_attachments(), pass.get(), target->get_attachments()[0].texture->get_width(), target->get_attachments()[0].texture->get_height());
			}
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
		case event_code::render_target_refresh_required:
			view->regenerate_render_targets();
			return false;
		default:
			return false;
		}
		return true;
	}
}