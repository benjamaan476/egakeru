#pragma once
#include "pch.h"
#include "renderer_types.h"
namespace egkr
{
	class renderer_frontend
	{
	public:
		using unique_ptr = std::unique_ptr<renderer_frontend>;
		API static unique_ptr create(backend_type type, const platform::shared_ptr& platform);

		renderer_frontend(backend_type type, const platform::shared_ptr& platform);

		API bool init();
		API void shutdown();
		API void on_resize(uint32_t width_, uint32_t height_);
		API void draw_frame(const render_packet& packet);
	private:
		renderer_backend::unique_ptr backend_{};
	};
}