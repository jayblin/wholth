#ifndef VK_DEVICE_H_
#define VK_DEVICE_H_

#include "vulkan/vulkan_core.h"
#include <span>
#include <vector>

namespace vk
{
    namespace device
    {
        struct QueueFamily
        {
            uint32_t index;
        };

        struct QueueFamilies
        {
            QueueFamily graphics;
        };

    };

    struct Device
    {
        Device();
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        Device(Device&& other)
        {
            *this = std::move(other);
        }

        Device& operator=(Device&& other);

        ~Device();

        auto init(
            const VkPhysicalDevice,
            const vk::device::QueueFamily&,
            const VkSurfaceKHR,
            const VkSurfaceCapabilitiesKHR&
        ) -> void;
        auto init_swapchain(
            VkSurfaceKHR,
            const VkSurfaceFormatKHR&,
            const VkSurfaceCapabilitiesKHR&
        ) -> VkSwapchainKHR;
        auto init_render_pass(const VkSurfaceFormatKHR&) -> VkRenderPass;
        auto init_descriptor_pool() -> VkDescriptorPool;
        auto init_images() -> std::span<VkImage>;
        auto init_image_acquired_semaphores() -> std::span<VkSemaphore>;
        auto init_render_complete_semaphores() -> std::span<VkSemaphore>;
        auto init_fences() -> std::span<VkFence>;
        auto init_command_pools(const vk::device::QueueFamily&) -> std::span<VkCommandPool>;
        auto init_command_buffers() -> std::span<VkCommandBuffer>;
        auto init_image_views(const VkSurfaceFormatKHR&) -> std::span<VkImageView>;
        auto init_framebuffers(const VkSurfaceCapabilitiesKHR&) -> std::span<VkFramebuffer>;

        VkDevice handle {VK_NULL_HANDLE};
        VkSwapchainKHR swapchain {VK_NULL_HANDLE};
        VkRenderPass render_pass {VK_NULL_HANDLE};
        VkPipelineCache pipeline_cache {VK_NULL_HANDLE};
        VkDescriptorPool descriptor_pool {VK_NULL_HANDLE};

        std::vector<VkImage> images {};
        std::vector<VkSemaphore> image_acquired_semaphores {};
        std::vector<VkSemaphore> render_complete_semaphores {};
        std::vector<VkFence> fences {};
        std::vector<VkCommandPool> command_pools {};
        std::vector<VkCommandBuffer> command_buffers {};
        std::vector<VkImageView> image_views {};
        std::vector<VkFramebuffer> framebuffers {};
    };
}

#endif // VK_DEVICE_H_
