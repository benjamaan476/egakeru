#pragma once
#include <renderer/render_view.h>
#include <resources/shader.h>

namespace egkr
{
	class render_view_world : public render_view
	{
	public:
		explicit render_view_world(const configuration& configuration);
		~render_view_world() override = default;
		bool on_create() override;
		bool on_destroy() override;
		void on_resize(uint32_t width, uint32_t height) override;
		render_view_packet on_build_packet(void* data) override;
		bool on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const override;
	private:
		shader::shader::shared_ptr shader_{};
		float4x4 projection_{};
		float4 ambient_colour_{};
		int render_mode{};
		debug_colour_shader_locations locations_{};
	};
}
