#ifndef UI_IMGUI_H_
#define UI_IMGUI_H_

#include "ui/glfw_window.hpp"
#include "vk/device.hpp"
#include "vk/instance.hpp"
#include "vulkan/vulkan_core.h"

namespace ui
{
    class Imgui
    {
    public:
        Imgui(
            const vk::Instance&,
            VkPhysicalDevice,
            const vk::Device&,
            vk::device::QueueFamily& queue_family,
            VkQueue queue,
            const VkSurfaceCapabilitiesKHR&,
            // @todo: do template param
            ui::GlfwWindow& window
        );

        ~Imgui();
    };
}

#endif // UI_IMGUI_H_
