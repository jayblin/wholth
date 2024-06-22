#ifndef VK_DESCRIPTOR_POOL_H_
#define VK_DESCRIPTOR_POOL_H_

#include "vulkan/vulkan_core.h"

namespace vk::descriptor_pool
{
    auto create(
        VkDescriptorPool* descriptor_pool,
        const VkDevice device
    ) -> void;
}

#endif // VK_DESCRIPTOR_POOL_H_
