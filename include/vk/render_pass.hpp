#ifndef VK_RENDER_PASS_H_
#define VK_RENDER_PASS_H_

#include "vulkan/vulkan_core.h"

namespace vk::render_pass
{
    auto create(
        VkRenderPass* render_pass,
        const VkDevice device,
        const VkSurfaceFormatKHR& surface_format
    ) -> void;
}


#endif // VK_RENDER_PASS_H_
