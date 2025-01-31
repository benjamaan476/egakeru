#pragma once
#include "pch.h"

#include "../platform.h"

class GLFWwindow;
namespace egkr
{
	class internal_platform final : public platform
	{
	public:
		using shared_ptr = std::shared_ptr<internal_platform>;
		static shared_ptr create(const platform::configuration& configuration);

		explicit internal_platform(const platform::configuration& configuration);
		~internal_platform() final;

		void shutdown() final;

		void pump() final;
		[[nodiscard]] bool is_running() const final;

		[[nodiscard]] std::chrono::nanoseconds get_time() const final;
		void sleep(std::chrono::nanoseconds time) const final;

		[[nodiscard]] egkr::vector<const char*> get_required_extensions() const final;	

		[[nodiscard]] void* get_window() const override;
		uint2 get_framebuffer_size() final;

		[[nodiscard]] static std::optional<dynamic_library> load_library(const std::string& library_name);
		static bool unload_library(dynamic_library& library);
		static bool load_function(const std::string& function_name, dynamic_library& library);
	private:

		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouse_callback(GLFWwindow* window, int button, int action, int mods);
		static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
		static void mouse_wheel_callback(GLFWwindow* window, double xpos, double ypos);
		static void on_close(GLFWwindow* window);
		static void on_resize(GLFWwindow* window, int width, int height);
		inline static bool is_initialised_{false};

		GLFWwindow* window_{};
		std::chrono::steady_clock::time_point startup_time_;
	};
}
