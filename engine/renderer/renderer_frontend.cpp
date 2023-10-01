#include "renderer_frontend.h"

#include "vulkan/renderer_vulkan.h"

namespace egkr
{
	renderer_frontend::unique_ptr renderer_frontend::create(backend_type type, const platform::shared_ptr& platform)
	{
		return std::make_unique<renderer_frontend>(type, platform);
	}

	renderer_frontend::renderer_frontend(backend_type type, const platform::shared_ptr& platform)
	{
		projection_ = glm::perspective(glm::radians(45.0F), platform->get_framebuffer_size().x / (float)platform->get_framebuffer_size().y, 0.1F, 1000.F);
		float4x4 view{1};
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
		return backend_->init();
	}

	void renderer_frontend::shutdown()
	{
		backend_->shutdown();
	}

	void renderer_frontend::on_resize(uint32_t width, uint32_t height)
	{
		projection_ = glm::perspective(glm::radians(45.0F), width / (float)height, near_clip_, far_clip_);
		backend_->resize(width, height);
	}

	void renderer_frontend::draw_frame(const render_packet& packet)
	{
		if (backend_->begin_frame(packet.delta_time))
		{
			static float angle = 0.F;
			angle -= 0.001F;
			backend_->update_global_state(projection_, view_, {}, {}, 0);

			float4x4 model{ 1 };
			model = glm::rotate(model, angle, { 0.F, 0.F, 1.F });

			geometry_render_data render_data{};
			render_data.model = model;

			backend_->update(render_data);

			backend_->end_frame();
		}
	}
	
	void renderer_frontend::set_view(const float4x4 & view)
{
		view_ = view;
}
}