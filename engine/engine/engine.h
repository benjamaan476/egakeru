#pragma once
#include "pch.h"

#include "platform/platform.h"
#include "application/application.h"
#include "event.h"


namespace egkr
{
	class engine
	{
	public:
		using unique_ptr = std::unique_ptr<engine>;
		static bool create(application::unique_ptr application);
		//Don't call this directly, only here to shutup clang
		explicit engine(application::unique_ptr application);

		static void run();
		void static shutdown();

	private:
		static bool on_event(event_code code, void* sender, void* listener, const event_context& context);
		static bool on_resize(event_code code, void* sender, void* listener, const event_context& context);
		bool is_initialised_{false};
		std::chrono::nanoseconds last_time_{};

	    bool limit_framerate_{false};
		std::chrono::milliseconds frame_time_{16ms};

		bool is_running_{};
		bool is_suspended_{};
		platform::shared_ptr platform_{};
		std::string name_{};
		application::unique_ptr application_{};
	};

}