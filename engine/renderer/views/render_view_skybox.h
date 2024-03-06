#pragma once
#include <pch.h>

#include <renderer/render_view.h>
#include <resources/shader.h>

namespace egkr::render_view
{

	class render_view_skybox : public render_view
	{
	public:
		explicit render_view_skybox(const configuration& configuration);
		~render_view_skybox() override = default;

		bool on_create() override;
		bool on_destroy() override;
		void on_resize(uint32_t width, uint32_t height) override;
		render_view_packet on_build_packet(void* data) override;
		bool on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const override;
		bool regenerate_attachment_target(uint32_t pass_index, const render_target::attachment& attachment) override;
	private:
		shader::shader::shared_ptr shader_{};
		float4x4 projection_{};

		uint16_t projection_location_{};
		uint16_t view_location_{};
		uint16_t cube_map_location_{};
	};
}
