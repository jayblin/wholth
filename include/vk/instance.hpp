#ifndef VK_INSTANCE_H_
#define VK_INSTANCE_H_

#include "vulkan/vulkan_core.h"
#include <utility>

namespace vk
{
    struct Instance
    {
        Instance();
        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;

        Instance(Instance&& other)
        {
            *this = std::move(other);
        }

        Instance& operator=(Instance&& other);

        ~Instance();

        auto init() -> void;

        template <typename Window>
        auto create_surface(Window&) -> VkSurfaceKHR;

        VkInstance handle {VK_NULL_HANDLE};
        VkSurfaceKHR surface {VK_NULL_HANDLE};
    };
}

#endif // VK_INSTANCE_H_
