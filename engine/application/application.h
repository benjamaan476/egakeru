#pragma once
#include "pch.h"

#include "platform/platform.h"
#include "game/game.h"
#include "event.h"

#include "resources/geometry.h"
#include "resources/mesh.h"

#include "renderer/renderer_frontend.h"

namespace egkr
{
	struct app_state
	{
		bool is_running{};
		bool is_suspended{};
		platform::shared_ptr platform{};
		uint32_t width_{};
		uint32_t height_{};
		std::string name{};
		game::unique_ptr game{};
		renderer_frontend::unique_ptr renderer{};
	};

	class application
	{
	public:
		using unique_ptr = std::unique_ptr<application>;
		API static bool create(game::unique_ptr game);
		//Don't call this directly, only here to shutup clang
		explicit application(game::unique_ptr game);

		API static void run();
		void static shutdown();

		const auto& get_state() const { return state_; }
	private:
		//void reginster_event();
		static bool on_event(event_code code, void* sender, void* listener, const event_context& context);
		static bool on_resize(event_code code, void* sender, void* listener, const event_context& context);
		static bool on_debug_event(event_code code, void* sender, void* listener, const event_context& context);
		bool is_initialised_{false};
		app_state state_{};
		std::chrono::nanoseconds last_time_{};

	    bool limit_framerate_{false};
		std::chrono::milliseconds frame_time_{16ms};

		egkr::vector<mesh::shared_ptr> meshes_;
		geometry::shared_ptr test_ui_geometry_{};
	};

}