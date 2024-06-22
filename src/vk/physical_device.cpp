#include "vk/physical_device.hpp"
#include "vk/device.hpp"
#include "vk/vk.hpp"

/* #define GLFW_INCLUDE_VULKAN */
#include "GLFW/glfw3.h"

std::vector<VkPhysicalDevice> vk::physical_device::list(VkInstance instance)
{
    uint32_t physical_device_count = 0;
    vk::check(
        vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr),
        "Couldn't get physical device count!"
    );

    std::vector<VkPhysicalDevice> physical_devices (physical_device_count);
    vk::check(
        vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()),
        "Couldn't populate physical device vector!"
    );

    return physical_devices;
}

std::tuple<
    VkPhysicalDevice,
    vk::device::QueueFamilies
> vk::physical_device::query_info(
    VkInstance instance,
    std::vector<VkPhysicalDevice>& physical_devices,
    VkSurfaceKHR surface
)
{
    /* vkEnumerateDeviceExtensionProperties() */
    /* vkGetPhysicalDeviceFeatures */

    /* VkDeviceDeviceMemoryReportCreateInfoEXT // looks cool and usefull */

    VkPhysicalDevice best_device = VK_NULL_HANDLE;
    vk::device::QueueFamilies queue_families {};
    int32_t max_score = 0;
    size_t i = 0;

    for (VkPhysicalDevice physical_device : physical_devices)
    {
        int32_t score = 1;
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physical_device, &props);

        if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == props.deviceType)
        {
            score += 10;
        }

        std::vector<VkQueueFamilyProperties> queue_family_properties {};

        uint32_t queue_family_property_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, nullptr);

        queue_family_properties.resize(queue_family_property_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, queue_family_properties.data());

        uint32_t j = 0;
        // Currently a device with most graphics queues is considered the
        // best.
        for (; j < queue_family_properties.size(); j++)
        {
            const auto& queue_family_prop = queue_family_properties[j];

            if (queue_family_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                score += 10;
            }

            VkBool32 supports_present;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, j, surface, &supports_present);

            if (!supports_present || !glfwGetPhysicalDevicePresentationSupport(instance, physical_device, j))
            {
                score = 0;
            }
        }

        if (score > max_score)
        {
            max_score = score;
            best_device = physical_devices[i];
            queue_families = {
                .graphics = {
                    // @todo: rethink
                    .index = j - 1,
                }
            };
        }

        i++;
    }

    if (VK_NULL_HANDLE == best_device)
    {
        vk::check(VK_ERROR_UNKNOWN, "Couldn't find acceptable physical device!");
    }

    return {best_device, queue_families};
}
