#pragma once
#include "pch.h"
#include "renderer_types.h"
#include "event.h"

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
		API void on_resize(uint32_t width, uint32_t height);
		API void draw_frame(const render_packet& packet);

		//TODO Hack
		API void set_view(const float4x4& view);

		auto get_backend_context() const { return backend_->get_context(); }

	private:
		renderer_backend::unique_ptr backend_{};

		float near_clip_{0.1F};
		float far_clip_{ 1000.F };
		float4x4 world_projection_{1.F};
		float4x4 world_view_{1.F};
		float4x4 ui_projection_{1.F};
		float4x4 ui_view_{1.F};
		//TODO temp
	};
}