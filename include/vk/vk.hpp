#ifndef VK_UTILS_H_
#define VK_UTILS_H_

#include "vulkan/vulkan_core.h"
#include "string_view"

namespace vk
{
    /* class VkException : public std::runtime_error */
    /* { */
    /* }; */

	void check(VkResult result, std::string_view msg);

	VkAllocationCallbacks* get_allocators();
}

#endif // VK_UTILS_H_
