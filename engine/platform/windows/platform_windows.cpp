#include "platform_windows.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "input.h"

namespace egkr
{
	platform_windows::~platform_windows()
	{
		shutdown();
		window_ = nullptr;
	}

	platform_windows::shared_ptr platform_windows::create()
	{
		if (!is_initialised_)
		{
			return std::make_shared<platform_windows>(); 
		}

		LOG_WARN("Already created platfrom");
		return nullptr;
	}

	bool platform_windows::startup(const platform_configuration& configuration)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window_ = glfwCreateWindow((int)configuration.width, (int)configuration.height, configuration.name.c_str(), nullptr, nullptr);

		if (window_ == nullptr)
		{
			LOG_FATAL("Failed to create glfw window");
			return false;
		}
		glfwMakeContextCurrent(window_);
		glfwSetWindowPos(window_, (int)configuration.start_x, (int)configuration.start_y);

		glfwSetKeyCallback(window_, &platform_windows::key_callback);

		is_initialised_ = true;
		return true;

	}
	void platform_windows::shutdown()
	{
		if (is_initialised_)
		{
			glfwSetWindowShouldClose(window_, 1);
			glfwDestroyWindow(window_);
			glfwTerminate();

		}

		is_initialised_ = false;
	}
	void platform_windows::pump()
	{
		glfwPollEvents();
	}
	bool platform_windows::is_running() const
	{
		return glfwWindowShouldClose(window_) == 0;
	}
	std::chrono::milliseconds platform_windows::get_time() const
	{
		const auto time = 10;
		return std::chrono::milliseconds{time};
	}
	void platform_windows::sleep(std::chrono::milliseconds time) const
	{
		std::this_thread::sleep_for(time);
	}
	void platform_windows::key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
	{
		if (window == nullptr)
		{
			LOG_WARN("Key event from wrong window");
			return;
		}
		auto pressed = action == GLFW_PRESS;
		input::process_key((egkr::key)key, pressed);
	}
}