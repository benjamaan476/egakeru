#pragma once
#include "pch.h"

#include <renderer/renderpass.h>
#include <renderer/camera.h>
#include <resources/mesh.h>
#include <resources/skybox.h>
#include <event.h>

namespace egkr
{
	namespace text
	{
		class ui_text;
	}

	struct render_view_packet;

	class render_view
	{
	public:
		enum class type
		{
			skybox,
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

		struct configuration
		{
			std::string name{};
			std::string custom_shader_name{};
			uint32_t width{};
			uint32_t height{};
			type type{};
			view_matrix_source view_source{};
			projection_matrix_source projection_source{};
			std::vector<renderpass::configuration> passes{};
		};

		using shared_ptr = std::shared_ptr<render_view>;
		static shared_ptr create(const configuration& configuration);

		explicit render_view(const configuration& configuration);
		virtual ~render_view() = default;

		virtual bool on_create() = 0;
		virtual bool on_destroy() = 0;
		virtual void on_resize(uint32_t width, uint32_t height) = 0;
		virtual render_view_packet on_build_packet(void* data) = 0;
		virtual bool on_render(const render_view_packet* render_view_packet, uint32_t frame_number, uint32_t render_target_index) const = 0;

		virtual bool regenerate_attachment_target(uint32_t pass_index, const render_target::attachment& attachment) = 0;

		void regenerate_render_targets();
		static bool on_event(egkr::event::code code, void* sender, void* listener, const event::context& context);

	protected:
		uint32_t id_{ invalid_32_id };
		std::string name_{};
		uint32_t width_{};
		uint32_t height_{};
		type type_{};
		uint32_t mode_{};
		std::vector<renderpass::renderpass::shared_ptr> renderpasses_{};
		std::string custom_shader_name_{};
		camera::shared_ptr camera_{};
	};

	struct render_view_packet
	{
		const render_view* render_view{};
		float4x4 view_matrix{ 1.F };
		float4x4 projection_matrix{ 1.F };

		float3 view_position{};
		float4 ambient_colour{ 1.F };

		egkr::vector<render_data> debug_render_data{};
		egkr::vector<render_data> render_data{};

		std::optional<std::string> custom_shader_name{};

		void* extended_data{};
	};

	struct skybox_packet_data
	{
		skybox::skybox::shared_ptr skybox{};
	};

	struct mesh_packet_data
	{
		egkr::vector<mesh::weak_ptr> meshes{};
		egkr::vector<render_data> debug_meshes{};
	};

	struct ui_packet_data
	{
		mesh_packet_data mesh_data;
		egkr::vector<std::weak_ptr<egkr::text::ui_text>> texts;
	};
}
