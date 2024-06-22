#ifndef UI_GLFW_WINDOW_H_
#define UI_GLFW_WINDOW_H_

/* #define GLFW_INCLUDE_VULKAN */
#include "GLFW/glfw3.h"
#include "ui/window.hpp"

namespace ui
{
    class GlfwWindow
    {
    public:
        ~GlfwWindow();

        GlfwWindow(const GlfwWindow&) = delete;
        GlfwWindow& operator=(const GlfwWindow&) = delete;

        GlfwWindow(GlfwWindow&& other);

        GlfwWindow& operator=(GlfwWindow&& other);

        auto should_close() -> bool;

        GlfwWindow(const ui::WindowOptions&);

        GLFWwindow* handle {nullptr};
    };
}
#endif // UI_GLFW_WINDOW_H_
