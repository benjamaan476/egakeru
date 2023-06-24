#include "Platform/Window.h"
#include "Platform/Windows/WindowsWindow.h"

Window::SharedPtr Window::create(const WindowProperties& properties, std::function<void(int, const char**)> dropCallback)
{
    return WindowsWindow::create(properties, dropCallback);
}
