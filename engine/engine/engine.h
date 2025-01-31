#pragma once
#include "pch.h"

#include "platform/platform.h"
#include "application/application.h"
#include "event.h"
#include "frame_data.h"
#include "renderer/renderer_frontend.h"


namespace egkr
{
	class engine
	{
	public:
		using unique_ptr = std::unique_ptr<engine>;
		static bool create(application::unique_ptr application);
		//Don't call this directly, only here to shutup clang
		explicit engine(application::unique_ptr application);

		void init();
		static void run();
		void static shutdown();

		[[nodiscard]] static const frame_data& get_frame_data();
		[[nodiscard]] const auto& get_renderer() const { return renderer_; }

		static const unique_ptr& get();

	private:
		static bool on_event(event::code code, void* sender, void* listener, const event::context& context);
		static bool on_resize(event::code code, void* sender, void* listener, const event::context& context);
		bool is_initialised_{false};
		std::chrono::nanoseconds last_time_{};

	    bool limit_framerate_{false};
		std::chrono::milliseconds frame_time_{16ms};

		bool is_running_{};
		bool is_suspended_{};
		std::string name_;
		application::unique_ptr application_;
		platform::shared_ptr platform_;
		renderer_frontend::unique_ptr renderer_;
		frame_data frame_data_{};
	};
}
