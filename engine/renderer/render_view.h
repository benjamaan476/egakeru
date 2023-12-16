#pragma once
#include "pch.h"

#include <renderer/renderpass.h>

namespace egkr
{
	namespace render_view
	{
		enum class type
		{
			world,
			ui
		};

		enum class view_matrix_source
		{
			scene_camera,
			ui_camera,
			light_camera
		};

		enum class projection_matrix_source
		{
			default_perspective,
			default_orthogonal
		};

		struct pass_configuration
		{
			std::string name{};
		};

		struct configuration
		{
			std::string name{};
			std::string custom_shader_name{};
			uint32_t width{};
			uint32_t height{};
			type type{};
			view_matrix_source view_source{};
			projection_matrix_source projection_source{};
			std::vector<pass_configuration> passes{};
		};

		struct render_view_packet;

		class render_view
		{
		public:
			using shared_ptr = std::shared_ptr<render_view>;
			static shared_ptr create(const renderer_backend* backend, const configuration& configuration);

			explicit render_view(const configuration& configuration);

			virtual bool on_create() = 0;
			virtual bool on_destroy() = 0;
			virtual void on_resize(uint32_t width, uint32_t height) = 0;
			virtual render_view_packet* on_build_packet(void* data) = 0;
			virtual bool on_render(render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) = 0;

		private:
			uint32_t id_{ invalid_32_id };
			std::string name_{};
			uint32_t width_{};
			uint32_t height_{};
			type type_{};

			std::vector<renderpass::renderpass::shared_ptr> renderpasses_{};
			std::optional<std::string> custom_shader_name_{};
		};

		struct render_view_packet
		{

		};
	}
}