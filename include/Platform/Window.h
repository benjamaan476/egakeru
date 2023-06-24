#pragma once

#include <vulkan/vulkan.hpp>

#include <string>
#include <memory>
#include <functional>
#include <tuple>

#include <GLFW/glfw3.h>

struct WindowProperties
{
	std::string title;
	uint32_t width;
	uint32_t height;
};

class Window
{
public:

	using SharedPtr = std::shared_ptr<Window>;
	using EventCallback = std::function<void(void)>;

	static SharedPtr create(const WindowProperties& properties, std::function<void(int, const char**)> dropCallback);
	virtual vk::SurfaceKHR createSurface(const vk::Instance& instance) = 0;
	virtual ~Window() = default;

	virtual void OnUpdate() const = 0;
	virtual bool isRunning() const = 0;
	
	virtual std::vector<const char*> GetRequiredExtensions() const = 0;

	virtual uint32_t GetWidth() const = 0;
	virtual uint32_t GetHeight() const = 0;

	virtual void SetEventCallback(const EventCallback& callback) = 0;
	virtual void SetVSync(bool enable) = 0;
	virtual bool IsVSync() const = 0;

	virtual std::pair<uint32_t, uint32_t> getFramebufferSize() const = 0;
	virtual void waitEvents() = 0;

	virtual GLFWwindow* getWindow() const = 0;
};

