#include "platform_windows.h"

//#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// #define GLFW_EXPOSE_NATIVE_WIN32
// #include <GLFW/glfw3native.h>

#include "systems/input.h"
#include "event.h"

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

	bool platform_windows::startup(const platform::configuration& platform_configuration)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window_ = glfwCreateWindow((int)platform_configuration.width_, (int)platform_configuration.height_, platform_configuration.name.c_str(), nullptr, nullptr);

		if (window_ == nullptr)
		{
			LOG_FATAL("Failed to create glfw window");
			return false;
		}
		glfwMakeContextCurrent(window_);
		glfwSetWindowPos(window_, (int)platform_configuration.start_x, (int)platform_configuration.start_y);

		glfwSetKeyCallback(window_, &platform_windows::key_callback);
		glfwSetMouseButtonCallback(window_, &platform_windows::mouse_callback);
		glfwSetScrollCallback(window_, &platform_windows::mouse_wheel_callback);
		glfwSetCursorPosCallback(window_, &platform_windows::mouse_move_callback);
		glfwSetWindowCloseCallback(window_, &platform_windows::on_close);
		glfwSetWindowSizeCallback(window_, &platform_windows::on_resize);

		startup_time_ = std::chrono::steady_clock::now();
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

	std::chrono::nanoseconds platform_windows::get_time() const
	{
		return std::chrono::steady_clock::now() - startup_time_;
	}

	void platform_windows::sleep(std::chrono::nanoseconds time) const
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
		auto pressed = action == GLFW_PRESS || action == GLFW_REPEAT;

		input::process_key((egkr::key)key, pressed);
	}

	void platform_windows::mouse_callback(GLFWwindow* window, int button, int action, int /*mods*/)
	{
		if (window == nullptr)
		{
			LOG_WARN("Mouse event from wrongg window");
			return;
		}

		bool pressed = action == GLFW_PRESS;
		input::process_button((egkr::mouse_button)button, pressed);
	}

	void platform_windows::mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (window == nullptr)
		{
			LOG_WARN("Mouse event from wrongg window");
			return;
		}

		input::process_mouse_move((int)xpos, (int)ypos);
	}

	void platform_windows::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		if (window == nullptr)
		{
			LOG_WARN("Mouse event from wrongg window");
			return;
		}

		input::process_mouse_wheel(xoffset, yoffset);
	}

	void platform_windows::on_close(GLFWwindow* window)
	{
		if (window == nullptr)
		{
			LOG_WARN("Key event from wrong window");
			return;
		}

		event::fire_event(event::code::quit, nullptr, {});
	}

	void platform_windows::on_resize(GLFWwindow* /*window*/, int width, int height)
	{
		event::context context{};
		context.set(0, (uint32_t)width);
		context.set(1, (uint32_t)height);
		event::fire_event(event::code::resize, nullptr, context);
	}

	egkr::vector<const char*> egkr::platform_windows::get_required_extensions() const
	{
		uint32_t extensionCount = 0;
		auto* extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		std::vector<const char*> extension(extensions, extensions + extensionCount);
		return  extension;
	}

	vk::SurfaceKHR platform_windows::create_surface(vk::Instance instance)
	{
		VkSurfaceKHR surface{};
		glfwCreateWindowSurface(instance, window_, nullptr, &surface);
		return { surface };
	}

	uint2 platform_windows::get_framebuffer_size()
	{
		int32_t width_{};
		int32_t height_{};
		glfwGetFramebufferSize(window_, &width_, &height_);

		return { (uint32_t)width_, (uint32_t)height_ };
	}
}
