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
		backend_->begin_frame(packet.delta_time);

		backend_->end_frame();
	}
}