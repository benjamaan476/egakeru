#include "platform_windows.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "input.h"
#include "event.h"

#include <systems/system.h>

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

		window_ = glfwCreateWindow((int)configuration.width_, (int)configuration.height_, configuration.name.c_str(), nullptr, nullptr);

		if (window_ == nullptr)
		{
			LOG_FATAL("Failed to create glfw window");
			return false;
		}
		glfwMakeContextCurrent(window_);
		glfwSetWindowPos(window_, (int)configuration.start_x, (int)configuration.start_y);

		glfwSetKeyCallback(window_, &platform_windows::key_callback);
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
		auto pressed = action == GLFW_PRESS | action == GLFW_REPEAT;

		input::process_key((egkr::key)key, pressed);
	}

	void platform_windows::on_close(GLFWwindow* window)
	{
		if (window == nullptr)
		{
			LOG_WARN("Key event from wrong window");
			return;
		}

		event::fire_event(event_code::quit, nullptr, {});
	}

	void platform_windows::on_resize(GLFWwindow* /*window*/, int width, int height)
	{
		event_context context{};
		const int array_size{ 4 };
		context = std::array<int32_t, array_size>{ width, height };
		event::fire_event(event_code::resize, nullptr, context);
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

	int2 platform_windows::get_framebuffer_size()
	{
		int32_t width_{};
		int32_t height_{};
		glfwGetFramebufferSize(window_, &width_, &height_);

		return { width_, height_ };
	}

}