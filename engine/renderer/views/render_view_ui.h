#pragma once
#include <renderer/render_view.h>

namespace egkr
{
	class shader;

	class render_view_ui : public render_view
	{
	public:
		render_view_ui(const configuration& configuration);
		~render_view_ui() override = default;

		bool on_create() override;
		bool on_destroy() override;
		void on_resize(uint32_t width, uint32_t height) override;
		render_view_packet on_build_packet(void* data) override;
		bool on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const override;
	private:

		std::shared_ptr<shader> shader_{};
		uint32_t diffuse_map_location_{};
		uint32_t diffuse_colour_location_{};
		uint32_t model_location_{};

		float near_clip_{};
		float far_clip_{};
		float4x4 view_{};
		float4x4 projection_{};
	};
}
