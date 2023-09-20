#pragma once
#include "pch.h"

#include "platform/platform.h"
#include "game/game.h"
#include "event.h"

namespace egkr
{
	struct app_state
	{
		bool is_running{};
		bool is_suspended{};
		platform::shared_ptr platform{};
		uint32_t width{};
		uint32_t height{};
		std::string name{};
		game::unique_ptr game{};
	};

	class application
	{
	public:
		using unique_ptr = std::unique_ptr<application>;
		API static unique_ptr create(game::unique_ptr game);
		//Don't call this directly, only here to shutup clang
		explicit application(game::unique_ptr game);

		static API void run();
		static void shutdown();

	private:
		//void reginster_event();
		static bool on_event(event_code code, void* sender, void* listener, const event_context& context);
		inline static bool is_initialised_{false};
		inline static app_state state_{};
	};
}