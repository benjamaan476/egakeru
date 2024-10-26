#pragma once
#include <renderer/render_view.h>
#include <resources/shader.h>

namespace egkr
{
	struct skybox_shader_locations
	{
		uint32_t projection{};
		uint32_t view{};
		uint32_t cubemap{};
	};

	class render_view_world : public render_view
	{
	public:
		explicit render_view_world(const configuration& configuration);
		~render_view_world() override = default;
		bool on_create() override;
		bool on_destroy() override;
		void on_resize(uint32_t width, uint32_t height) override;
		render_view_packet on_build_packet(void* data, const camera::shared_ptr& camera, viewport* viewport) override;
		bool on_render(render_view_packet* render_view_packet, const frame_data& frame_data) const override;
	private:
		shader::shader::shared_ptr skybox_shader_{};
		shader::shader::shared_ptr shader_{};
		float4 ambient_colour_{};
		int render_mode{};
		debug_colour_shader_locations locations_{};
		skybox_shader_locations skybox_locations_{};
	};
}
