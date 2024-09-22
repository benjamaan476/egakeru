#include "render_view.h"
#include "renderer/views/render_view_ui.h"
#include "renderer/views/render_view_world.h"
#include "renderer/views/render_view_editor.h"
#include <renderer/renderer_frontend.h>
#include <systems/camera_system.h>

namespace egkr
{
	render_view::shared_ptr render_view::create(const configuration& configuration)
	{
		if (configuration.view_type == type::world)
		{
			return std::make_shared<render_view_world>(configuration);
		}
		else if (configuration.view_type == type::ui)
		{
			return std::make_shared<render_view_ui>(configuration);
		}
		else if (configuration.view_type == type::editor)
		{
			return std::make_shared<render_view_editor>(configuration);
		}
		else
		{
			LOG_ERROR("Unrecognized view type");
			return nullptr;
		}
	}

	render_view::render_view(const configuration& view_configuration)
		: name_{ view_configuration.name },
		width_{ view_configuration.width },
		height_{ view_configuration.height },
		type_{ view_configuration.view_type },
		custom_shader_name_{ view_configuration.custom_shader_name }
	{
		camera_ = camera_system::get_default();

		for (const auto& pass : view_configuration.passes)
		{
			auto renderpass = renderpass::renderpass::create(pass);
			renderpasses_.push_back(renderpass);
		}

		regenerate_render_targets();
	}

	void render_view::regenerate_render_targets()
	{
		for (const auto& pass : renderpasses_)
		{
			for (uint8_t i{ 0U }; i < pass->get_render_target_count(); ++i)
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
							attachment.texture_attachment = renderer->get_backend()->get_window_attachment(i);
						}
						else if (attachment.type == render_target::attachment_type::depth)
						{
							attachment.texture_attachment = renderer->get_backend()->get_depth_attachment(i);
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

				target = render_target::render_target::create(target->get_attachments(), pass.get(), target->get_attachments()[0].texture_attachment->get_width(), target->get_attachments()[0].texture_attachment->get_height());
			}
		}
	}

	bool render_view::on_event(event::code code, void* /*sender*/, void* listener, const event::context& context)
	{
		render_view* view = (render_view*)listener;

		switch (code)
		{
		case event::code::render_mode:
		{
			context.get(0, view->mode_);
		} break;
		case event::code::render_target_refresh_required:
			view->regenerate_render_targets();
			break;
		default:
			return false;
		}
		return false;
	}
}
