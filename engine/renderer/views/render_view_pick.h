#pragma once
#include <renderer/render_view.h>
#include <resources/shader.h>

namespace egkr::render_view
{
	struct pick_shader_locations
	{
		uint16_t projection{};
		uint16_t view{};
		uint16_t model{};
	};
	
	struct pick_shader_info
	{
		shader::shader::shared_ptr shader{};
		renderpass::renderpass::shared_ptr renderpass{};
		uint16_t id_colour_location{};
		uint16_t model_location{};
		uint16_t projection_location{};
		uint16_t view_location{};
		float4x4 projection{};
		float4x4 view{};
		float near_clip{};
		float far_clip{};
		float fov{};

	};

	class render_view_pick : public render_view
	{
	public:
		explicit render_view_pick(const configuration& configuration);

		bool on_create() override;
		bool on_destroy() override;
		void on_resize(uint32_t width, uint32_t height) override;
		render_view_packet on_build_packet(void* data) override;
		bool on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const override;
		bool regenerate_attachment_target(uint32_t pass_index, const render_target::attachment& attachment) override;

		void acquire_shader_instances();
		void release_shader_instances();
		static bool on_mouse_move(event_code code, void* sender, void* listener, const event_context& context);
	private:
		pick_shader_info ui_shader_info{};
		pick_shader_info world_shader_info{};
		texture::texture* colour_target_attachment{};
		texture::texture* depth_target_attachment{};
		egkr::vector<bool> instance_updated{};
		int16_t mouse_x{};
		int16_t mouse_y{};
	};
}
