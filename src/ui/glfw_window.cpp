#include "ui/internal/window.hpp"
#include "ui/internal/glfw_window.hpp"
#include "vk/instance.hpp"
#include "vk/vk.hpp"
#include "backends/imgui_impl_glfw.h"
#include <stdexcept>
#include <utility>


ui::internal::GlfwWindow::~GlfwWindow()
{
    if (nullptr != handle)
    {
        glfwDestroyWindow(handle);
    }
}

ui::internal::GlfwWindow::GlfwWindow(ui::internal::GlfwWindow&& other)
{
    *this = std::move(other);
}

ui::internal::GlfwWindow& ui::internal::GlfwWindow::operator=(ui::internal::GlfwWindow&& other)
{
    if (this != &other)
    {
        this->handle = other.handle;

        other.handle = nullptr;
    }

    return *this;
}

bool ui::internal::GlfwWindow::should_close()
{
    return glfwWindowShouldClose(handle);
}

ui::internal::GlfwWindow::GlfwWindow(const ui::internal::WindowOptions& options)
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
ui::internal::WindowFactory<ui::internal::GlfwWindow>::WindowFactory()
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
ui::internal::WindowFactory<ui::internal::GlfwWindow>::~WindowFactory()
{
    glfwTerminate();
}

template<>
VkSurfaceKHR ui::internal::SurfaceFactory<ui::internal::GlfwWindow>::create(VkInstance instance, ui::internal::GlfwWindow& window)
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
VkSurfaceKHR vk::Instance::create_surface(ui::internal::GlfwWindow& window)
{
    if (VK_NULL_HANDLE == this->surface) {
        this->surface = (ui::internal::SurfaceFactory<ui::internal::GlfwWindow>{}).create(this->handle, window);
    }

    return this->surface;
}
