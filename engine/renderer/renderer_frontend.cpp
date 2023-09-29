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
	void renderer_frontend::on_resize(uint32_t width_, uint32_t height_)
	{
		backend_->resize(width_, height_);
	}
	void renderer_frontend::draw_frame(const render_packet& packet)
	{
		if (backend_->begin_frame(packet.delta_time))
		{

			float4x4 projection = glm::perspective(45.0F / 180.F * std::numbers::pi_v<float>, 800.F / 600.F, 0.1F, 1000.F);

			static float z = 30.F;
			static float angle = 0.F;
			angle -= 0.001F;
			z += 0.1F;
			float4x4 view{1};
			view = glm::translate(view, { 0.F, 0.F, z });

			view = glm::inverse(view);
			backend_->update_global_state(projection, view, {}, {}, 0);

			float4x4 model{ 1 };
			model = glm::rotate(model, angle, { 0.F, 0.F, 1.F });

			backend_->update(model);

			backend_->end_frame();
		}
	}
}