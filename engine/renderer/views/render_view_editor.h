#pragma once
#include <renderer/render_view.h>
#include <resources/shader.h>
#include <editor_gizmo.h>

namespace egkr
{

	class render_view_editor : public render_view
	{
	public:
		explicit render_view_editor(const configuration& configuration);
		~render_view_editor() override = default;
		bool on_create() override;
		bool on_destroy() override;
		void on_resize(uint32_t width, uint32_t height) override;
		render_view_packet on_build_packet(void* data, viewport* viewport) override;
		bool on_render(render_view_packet* render_view_packet, const frame_data& frame_data) const override;

	private:
		shader::shader::shared_ptr shader_{};
		float4 ambient_colour_{};
		int render_mode{};
		debug_colour_shader_locations locations_{};
		editor::gizmo gizmo_{};
	};
}
