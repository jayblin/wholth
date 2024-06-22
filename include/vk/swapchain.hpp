#ifndef VK_SWAPCHAIN_H_
#define VK_SWAPCHAIN_H_

#include "vulkan/vulkan_core.h"
#include <string_view>
#include <vector>

namespace vk::swapchain
{
    auto create(
        VkSwapchainKHR* swapchain,
        const VkDevice device,
        const VkSurfaceKHR surface,
        const VkSurfaceFormatKHR& surface_format,
        const VkSurfaceCapabilitiesKHR& surface_capabilities
    ) -> void;

    auto get_images(
        const VkDevice device,
        const VkSwapchainKHR swapchain
    ) -> std::vector<VkImage>;
}

#endif // VK_SWAPCHAIN_H_
