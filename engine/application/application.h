#pragma once
#include "pch.h"
#include "keymap.h"
#include "engine/engine_configuration.h"

namespace egkr
{
	class engine;
	struct render_packet;

	struct application_data : public frame_geometry_data
	{

	};

	class application
	{
	public:

		using unique_ptr = std::unique_ptr<application>;
		API explicit application(engine_configuration configuration);

		API virtual ~application()  = default;

		virtual bool init() = 0;
		virtual void update(const frame_data& frame_data) = 0;
		virtual void render(render_packet* render_packet, const frame_data& frame_data) = 0;
		virtual bool resize(uint32_t width, uint32_t height) = 0;

		virtual bool boot() = 0;
		virtual bool shutdown() = 0;

		[[nodiscard]] const auto& get_engine_configuration() const	{ return engine_configuration_; }
		[[nodiscard]] const auto& get_font_system_configuration() const { return font_system_configuration_; }
		[[nodiscard]] const auto& get_render_view_configuration() const { return render_view_configuration_; }
		[[nodiscard]] const auto& get_camera() const { return camera_; }

		[[nodiscard]] const auto& get_console_keymap() const { return console_keymap; }
		[[nodiscard]] auto& get_console_keymap() { return console_keymap; }

		void set_engine(engine* engine);
	protected:
		[[nodiscard]] auto* get_engine() const { return engine_;}
		font_system::configuration font_system_configuration_{};
		egkr::vector<render_view::configuration> render_view_configuration_{};

		engine* engine_;
		uint32_t width_{};
		uint32_t height_{};

		egkr::camera::shared_ptr camera_{};
		keymap console_keymap{};

	private:

		engine_configuration engine_configuration_{};
	};

	static application::unique_ptr application_{};
}