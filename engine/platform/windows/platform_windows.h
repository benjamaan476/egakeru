#pragma once
#include "pch.h"

#include "../platform.h"

class GLFWwindow;
namespace egkr
{
	class platform_windows final : public platform
	{
	public:
		using shared_ptr = std::shared_ptr<platform_windows>;

		~platform_windows() final;

		static shared_ptr create();
		bool startup(const platform_configuration& configuration) final;
		void shutdown() final;

		void pump() final;
		bool is_running() const final;

		std::chrono::milliseconds get_time() const final;
		void sleep(std::chrono::milliseconds time) const final;
	private:

		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		inline static bool is_initialised_{false};

		GLFWwindow* window_{};
	};
}