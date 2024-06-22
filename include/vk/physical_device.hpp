#ifndef VK_PHYSICAL_DEVICE_H_
#define VK_PHYSICAL_DEVICE_H_

#include "vk/device.hpp"
#include <tuple>
#include <vector>

namespace vk::physical_device
{
    auto list(VkInstance instance) -> std::vector<VkPhysicalDevice>;

    auto query_info(
        VkInstance,
        std::vector<VkPhysicalDevice>&,
        VkSurfaceKHR
    ) -> std::tuple<VkPhysicalDevice, vk::device::QueueFamilies>;
}

#endif // VK_PHYSICAL_DEVICE_H_
