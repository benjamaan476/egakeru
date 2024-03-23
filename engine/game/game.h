#pragma once
#include "pch.h"
#include "keymap.h"
#include "application/application_configuration.h"

namespace egkr
{
	class application;
	struct render_packet;

	class game
	{
	public:
		using unique_ptr = std::unique_ptr<game>;
		API explicit game(application_configuration configuration);

		API virtual ~game()  = default;

		virtual bool init() = 0;
		virtual void update(double delta_time) = 0;
		virtual void render(render_packet* render_packet, double delta_time) = 0;
		virtual bool resize(uint32_t width, uint32_t height) = 0;

		virtual bool boot() = 0;
		virtual bool shutdown() = 0;

		[[nodiscard]] const auto& get_application_configuration() const	{ return application_configuration_; }
		[[nodiscard]] const auto& get_font_system_configuration() const { return font_system_configuration_; }
		[[nodiscard]] const auto& get_render_view_configuration() const { return render_view_configuration_; }
		[[nodiscard]] const auto& get_camera() const { return camera_; }
		[[nodiscard]] double get_delta_time() { return delta_time_; }

		[[nodiscard]] const auto& get_console_keymap() const { return console_keymap; }
		[[nodiscard]] auto& get_console_keymap() { return console_keymap; }

		void set_application(application* app);
	protected:
		[[nodiscard]] auto* get_application() const { return application_;}
		font_system::configuration font_system_configuration_{};
		egkr::vector<render_view::configuration> render_view_configuration_{};

		application* application_;
		uint32_t width_{};
		uint32_t height_{};

		egkr::camera::shared_ptr camera_{};
		double delta_time_{};
		keymap console_keymap{};

	private:

		application_configuration application_configuration_{};
	};

	static game::unique_ptr game_{};
}