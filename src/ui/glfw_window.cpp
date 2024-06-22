#include "ui/window.hpp"
#include "ui/glfw_window.hpp"
#include "vk/instance.hpp"
#include "vk/vk.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <stdexcept>
#include <utility>


ui::GlfwWindow::~GlfwWindow()
{
    if (nullptr != handle)
    {
        glfwDestroyWindow(handle);
    }
}

ui::GlfwWindow::GlfwWindow(ui::GlfwWindow&& other)
{
    *this = std::move(other);
}

ui::GlfwWindow& ui::GlfwWindow::operator=(ui::GlfwWindow&& other)
{
    if (this != &other)
    {
        this->handle = other.handle;

        other.handle = nullptr;
    }

    return *this;
}

bool ui::GlfwWindow::should_close()
{
    return glfwWindowShouldClose(handle);
}

ui::GlfwWindow::GlfwWindow(const ui::WindowOptions& options)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    handle = glfwCreateWindow(options.width, options.height, "Window Title", nullptr, nullptr);

    glfwSetMouseButtonCallback(handle, ImGui_ImplGlfw_MouseButtonCallback);
    glfwSetCursorEnterCallback(handle, ImGui_ImplGlfw_CursorEnterCallback);
    glfwSetCursorPosCallback(handle, ImGui_ImplGlfw_CursorPosCallback);
    
    glfwSetWindowFocusCallback(handle, ImGui_ImplGlfw_WindowFocusCallback);
    glfwSetScrollCallback(handle, ImGui_ImplGlfw_ScrollCallback);
    glfwSetKeyCallback(handle, ImGui_ImplGlfw_KeyCallback);
    glfwSetCharCallback(handle, ImGui_ImplGlfw_CharCallback);
    glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
}


template<>
ui::WindowFactory<ui::GlfwWindow>::WindowFactory()
{
    if (!glfwInit())
    {
        /* report_error("GLFW: Initialization failed!"); */
        throw std::runtime_error("GLFW: Initialization failed!");
    }

    if (!glfwVulkanSupported())
    {
        /* report_error("GLFW: Vulkan is not supported!"); */
        throw std::runtime_error("GLFW: Vulkan is not supported!");
    }
}

template<>
ui::WindowFactory<ui::GlfwWindow>::~WindowFactory()
{
    glfwTerminate();
}

template<>
VkSurfaceKHR ui::SurfaceFactory<ui::GlfwWindow>::create(VkInstance instance, ui::GlfwWindow& window)
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    vk::check(
        glfwCreateWindowSurface(
            instance,
            window.handle,
            vk::get_allocators(),
            &surface
        ),
        "Couldn't create GLFW window surface!"
    );

    return surface;
}

template<>
VkSurfaceKHR vk::Instance::create_surface(ui::GlfwWindow& window)
{
    if (VK_NULL_HANDLE == this->surface) {
        this->surface = (ui::SurfaceFactory<ui::GlfwWindow>{}).create(this->handle, window);
    }

    return this->surface;
}
