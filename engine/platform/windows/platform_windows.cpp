#include "platform_windows.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <Windows.h>

#include "systems/input.h"
#include "event.h"

namespace egkr
{
	internal_platform::~internal_platform()
	{
		shutdown();
		window_ = nullptr;
	}

	internal_platform::shared_ptr internal_platform::create()
	{
		if (!is_initialised_)
		{
			return std::make_shared<internal_platform>(); 
		}

		LOG_WARN("Already created platfrom");
		return nullptr;
	}

	bool internal_platform::startup(const platform::configuration& platform_configuration)
	{
		if(!glfwInit())
		{
			LOG_ERROR("Failed to initialise platform");
			return false;
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window_ = glfwCreateWindow((int)platform_configuration.width_, (int)platform_configuration.height_, platform_configuration.name.c_str(), nullptr, nullptr);

		if (window_ == nullptr)
		{
			LOG_FATAL("Failed to create glfw window");
			return false;
		}
		glfwMakeContextCurrent(window_);
		glfwSetWindowPos(window_, (int)platform_configuration.start_x, (int)platform_configuration.start_y);

		glfwSetKeyCallback(window_, &internal_platform::key_callback);
		glfwSetMouseButtonCallback(window_, &internal_platform::mouse_callback);
		glfwSetScrollCallback(window_, &internal_platform::mouse_wheel_callback);
		glfwSetCursorPosCallback(window_, &internal_platform::mouse_move_callback);
		glfwSetWindowCloseCallback(window_, &internal_platform::on_close);
		glfwSetWindowSizeCallback(window_, [](GLFWwindow*, int width, int height)
				{
				event::context context{};
				context.set(0, (uint32_t)width);
				context.set(1, (uint32_t)height);
				event::fire_event(event::code::resize, nullptr, context);
				});

		startup_time_ = std::chrono::steady_clock::now();
		is_initialised_ = true;
		return true;
	}

	void internal_platform::shutdown()
	{
		if (is_initialised_)
		{
			glfwSetWindowShouldClose(window_, 1);
			glfwDestroyWindow(window_);
			glfwTerminate();

		}

		is_initialised_ = false;
	}

	void internal_platform::pump()
	{
		glfwPollEvents();
	}

	bool internal_platform::is_running() const
	{
		return glfwWindowShouldClose(window_) == 0;
	}

	std::chrono::nanoseconds internal_platform::get_time() const
	{
		return std::chrono::steady_clock::now() - startup_time_;
	}

	void internal_platform::sleep(std::chrono::nanoseconds time) const
	{
		std::this_thread::sleep_for(time);
	}

	void internal_platform::key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
	{
		if (window == nullptr)
		{
			LOG_WARN("Key event from wrong window");
			return;
		}
		auto pressed = action == GLFW_PRESS || action == GLFW_REPEAT;

		input::process_key((egkr::key)key, pressed);
	}

	void internal_platform::mouse_callback(GLFWwindow* window, int button, int action, int /*mods*/)
	{
		if (window == nullptr)
		{
			LOG_WARN("Mouse event from wrongg window");
			return;
		}

		bool pressed = action == GLFW_PRESS;
		input::process_button((egkr::mouse_button)button, pressed);
	}

	void internal_platform::mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (window == nullptr)
		{
			LOG_WARN("Mouse event from wrongg window");
			return;
		}

		input::process_mouse_move((int)xpos, (int)ypos);
	}

	void internal_platform::mouse_wheel_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		if (window == nullptr)
		{
			LOG_WARN("Mouse event from wrongg window");
			return;
		}

		input::process_mouse_wheel(xoffset, yoffset);
	}

	void internal_platform::on_close(GLFWwindow* window)
	{
		if (window == nullptr)
		{
			LOG_WARN("Key event from wrong window");
			return;
		}

		event::fire_event(event::code::quit, nullptr, {});
	}

	void internal_platform::on_resize(GLFWwindow* /*window*/, int width, int height)
	{
		event::context context{};
		context.set(0, (uint32_t)width);
		context.set(1, (uint32_t)height);
		event::fire_event(event::code::resize, nullptr, context);
	}

	egkr::vector<const char*> egkr::internal_platform::get_required_extensions() const
	{
		uint32_t extensionCount = 0;
		auto* extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		std::vector<const char*> extension(extensions, extensions + extensionCount);
		return  extension;
	}

	vk::SurfaceKHR internal_platform::create_surface(vk::Instance instance)
	{
		VkSurfaceKHR surface{};
		glfwCreateWindowSurface(instance, window_, nullptr, &surface);
		return { surface };
	}

	uint2 internal_platform::get_framebuffer_size()
	{
		int32_t width_{};
		int32_t height_{};
		glfwGetFramebufferSize(window_, &width_, &height_);

		return { (uint32_t)width_, (uint32_t)height_ };
	}

	// std::expected<platform::dynamic_library, void> internal_platform::load_library(const std::string& library_name) const
	// {
	// 	if(library_name.empty())
	// 	{
	// 		LOG_ERROR("Invalid library name, cannot load");
	// 		return {};
	// 	}
	//
	// 	std::string filename = std::format("%s.dll", library_name);
	//
	// 	HMODULE library = LoadLibraryA(filename.c_str());
	// 	if(!library)
	// 	{
	// 		LOG_ERROR("Could not load library: {}", library_name);
	// 		return {};
	// 	}
	//
	// 	return {.size = sizeof(HMODULE), .internal_data = library, .library_name = library_name, .filename = filename};
	// }
	//
	// bool internal_platform::unload_library(dynamic_library& library) const
	// {
	// 	if(library.internal_data == nullptr)
	// 	{
	// 		LOG_ERROR("Library already unloaded: {}", library.library_name);
	// 		return false;
	// 	}
	//
	// 	if(FreeLibrary((HMODULE)library.internal_data) == 0)
	// 	{
	// 		LOG_ERROR("Couldn't free library: {}", library.library_name);
	// 		return false;
	// 	}
	//
	// 	library = {};
	// }
	//
	// bool internal_platform::load_function(const std::string& function_name, dynamic_library& library) const
	// {
	// 	if(library.internal_data == nullptr)
	// 	{
	// 		LOG_ERROR("Attempted to load function from invalid library");
	// 		return false;
	// 	}
	// 	
	// 	FARPROC addr = GetProcAddress((HMODULE)library.interal_data, function_name.c_str());
	// 	if(!addr)
	// 	{
	// 		LOG_ERROR("Could not load function {} from {}", function_name, library.library_name);
	// 		return false;
	// 	}
	// 	library.functions.emplace_back(function_name, addr);
	// 	return true;
	//
	// }
}
