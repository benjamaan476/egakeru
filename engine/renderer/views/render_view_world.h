#pragma once
#include <renderer/render_view.h>
#include <renderer/camera.h>

namespace egkr::render_view
{
	class render_view_world : public render_view
	{
	public:
		render_view_world(const renderer_frontend* renderer, const configuration& configuration);
		~render_view_world() override = default;
		bool on_create() override;
		bool on_destroy() override;
		void on_resize(uint32_t width, uint32_t height) override;
		render_view_packet on_build_packet(void* data) override;
		bool on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const override;

	private:
		uint32_t shader_id_{invalid_32_id};
		float fov_{90.F};
		float near_clip_{0.01F};
		float far_clip_{1000.F};
		
		float4x4 projection_{};
		camera::shared_ptr camera_{};
		float4 ambient_colour_{};
		int render_mode{};
	};
}
