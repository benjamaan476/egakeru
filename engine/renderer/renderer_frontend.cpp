#include "renderer_frontend.h"
#include "event.h"

#include "systems/texture_system.h"
#include "systems/geometry_system.h"
#include "vulkan/renderer_vulkan.h"

namespace egkr
{
	renderer_frontend::unique_ptr renderer_frontend::create(backend_type type, const platform::shared_ptr& platform)
	{
		return std::make_unique<renderer_frontend>(type, platform);
	}

	renderer_frontend::renderer_frontend(backend_type type, const platform::shared_ptr& platform)
	{
		const auto& size = platform->get_framebuffer_size();
		world_projection_ = glm::perspective(glm::radians(45.0F), size.x / (float)size.y, 0.1F, 1000.F);
		ui_projection_ = glm::ortho(0.F, (float)size.x, (float)size.y, 0.F, -100.F, 100.F);

		float4x4 view{ 1 };
		view = glm::translate(view, { 0.F, 0.F, 30.F });
		view = glm::inverse(view);
		switch (type)
		{
		case backend_type::vulkan:
			backend_ = renderer_vulkan::create(platform);
			break;
		case backend_type::opengl:
		case backend_type::directx:
		default:
			LOG_ERROR("Unsupported renderer backend chosen");
			break;
		}

	}

	bool renderer_frontend::init()
	{
		auto backen_init = backend_->init();

		return backen_init;
	}

	void renderer_frontend::shutdown()
	{
		backend_->shutdown();
	}

	void renderer_frontend::on_resize(uint32_t width, uint32_t height)
	{
		world_projection_ = glm::perspective(glm::radians(45.0F), width / (float)height, near_clip_, far_clip_);
		ui_projection_ = glm::ortho(0.f, (float)width, (float)height, 0.F, -100.F, 100.F);
		backend_->resize(width, height);
	}

	void renderer_frontend::draw_frame(const render_packet& packet)
	{
		if (backend_->begin_frame(packet.delta_time))
		{
			if (backend_->begin_renderpass(builtin_renderpass::world))
			{
				backend_->update_world_state(world_projection_, world_view_, {}, {}, 0);


				for (const auto& render_data : packet.world_geometry_data)
				{
					backend_->draw_geometry(render_data);
				}

				if (!backend_->end_renderpass(builtin_renderpass::world))
				{
					LOG_ERROR("Failed to end world renderpass");
					return;
				}


				if (backend_->begin_renderpass(builtin_renderpass::ui))
				{
					backend_->update_ui_state(ui_projection_, ui_view_, {}, {}, 0);

					for (const auto& render_data : packet.ui_geometry_data)
					{
						backend_->draw_geometry(render_data);
					}

					if (!backend_->end_renderpass(builtin_renderpass::ui))
					{
						LOG_ERROR("Failed to end ui renderpass");
						return;
					}
				}
			}

			backend_->end_frame();
		}
	}

	void renderer_frontend::set_view(const float4x4& view)
	{
		world_view_ = view;
	}
}