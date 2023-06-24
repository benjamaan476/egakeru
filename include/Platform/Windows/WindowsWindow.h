#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>

#include "../Window.h"

class WindowsWindow : public Window
{
public:
	static Window::SharedPtr create(const WindowProperties& properties, std::function<void(int, const char**)>  dropCallback);
	vk::SurfaceKHR createSurface(const vk::Instance& instance) override;
	~WindowsWindow() override;

	void OnUpdate() const override;
	bool isRunning() const override;

	std::vector<const char*> GetRequiredExtensions() const override;

	uint32_t GetWidth() const override { return data.width; }
	uint32_t GetHeight() const override { return data.height; }

	void SetEventCallback(const EventCallback& callback) override { data.callback = callback; }
	void SetVSync(bool enable) override;
	bool IsVSync() const override;

	std::pair<uint32_t, uint32_t> getFramebufferSize() const override;
	void waitEvents() override;

	GLFWwindow* getWindow() const override { return window; }
private:
	explicit WindowsWindow(const WindowProperties& properties, std::function<void(int, const char**)>  dropCallback);
	virtual void Shutdown();

private:
	static inline bool isGLFWInitialised = false;

	GLFWwindow* window;
	struct WindowData
	{
		std::string title;
		uint32_t width;
		uint32_t height;
		EventCallback callback;

		bool VSync;
	} data{};
};

