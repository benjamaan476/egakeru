#pragma once
#include "pch.h"

#include "platform/platform.h"
#include "game/game.h"
#include "event.h"

#include "resources/geometry.h"
#include "resources/mesh.h"
#include "resources/light.h"
#include "resources/skybox.h"
#include "resources/ui_text.h"

#include <debug/debug_box3d.h>
#include <debug/debug_grid.h>

#include "renderer/renderer_frontend.h"

namespace egkr
{
	class application
	{
	public:
		using unique_ptr = std::unique_ptr<application>;
		static bool create(game::unique_ptr game);
		//Don't call this directly, only here to shutup clang
		explicit application(game::unique_ptr game);

		static void run();
		void static shutdown();

	private:
		//void reginster_event();
		static bool on_event(event_code code, void* sender, void* listener, const event_context& context);
		static bool on_resize(event_code code, void* sender, void* listener, const event_context& context);
		bool is_initialised_{false};
	    bool limit_framerate_{false};
		bool is_running_{};
		bool is_suspended_{};

		std::chrono::nanoseconds last_time_{};
		std::chrono::milliseconds frame_time_{16ms};

		platform::shared_ptr platform_{};
		std::string name_{};
		game::unique_ptr game_{};
	};

}