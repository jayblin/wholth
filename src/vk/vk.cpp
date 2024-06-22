#include "vk/vk.hpp"
#include "fmt/core.h"
#include <stdexcept>

void vk::check(VkResult result, std::string_view msg = "")
{
    if (VK_SUCCESS != result)
    {
        throw std::runtime_error(
            fmt::format("{}: {}", msg, static_cast<int>(result))
        );
        /* throw VkException(std::string(msg)); */
    }
}

VkAllocationCallbacks* vk::get_allocators()
{
	return nullptr;
}
