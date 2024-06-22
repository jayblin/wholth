#ifndef VK_SURFFACE_H_
#define VK_SURFFACE_H_

#include "vk/vk.hpp"

namespace vk::surface
{
    class Capabilities
    {
    };
}

namespace vk
{
    class Surface
    {
    public:
        Surface();
        Surface(const Surface&) = delete;
        Surface& operator=(const Surface&) = delete;

        Surface(Surface&& other)
        {
            *this = std::move(other);
        }

        // @todo: implement
        Surface& operator=(Surface&& other);

        ~Surface();

        auto init(VkPhysicalDevice physical_device) -> void
        {
            vk::check(
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                    physical_device,
                    m_handle,
                    &m_capabilities
                ),
                "Couldn't get surface capabilities!"
            );
        }

        auto capabilities() const -> const VkSurfaceCapabilitiesKHR&
        {
            return m_capabilities;
        }

        auto handle() const -> VkSurfaceKHR
        {
            return this->m_handle;
        }

    private:
        VkSurfaceKHR m_handle {VK_NULL_HANDLE};
        VkSurfaceCapabilitiesKHR m_capabilities;
    };
}

#endif // VK_SURFFACE_H_
