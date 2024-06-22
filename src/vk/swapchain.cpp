#include "vk/swapchain.hpp"
#include "vk/vk.hpp"

void vk::swapchain::create(
    VkSwapchainKHR *swapchain,
    const VkDevice device,
    const VkSurfaceKHR surface,
    const VkSurfaceFormatKHR &surface_format,
    const VkSurfaceCapabilitiesKHR &surface_capabilities
)
{
    VkSwapchainCreateInfoKHR swapchain_create_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount = surface_capabilities.minImageCount,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = surface_capabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr,

    };
    vk::check(
        vkCreateSwapchainKHR(device, &swapchain_create_info, vk::get_allocators(), swapchain),
        "Couln't create swapchain!"
    );
}

std::vector<VkImage> vk::swapchain::get_images(
    const VkDevice device,
    const VkSwapchainKHR swapchain
)
{
    uint32_t image_count;
    vk::check(
        vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr),
        "Couldn't get count of swapchain images!"
    );

    std::vector<VkImage> images (image_count, VK_NULL_HANDLE);

    vk::check(
        vkGetSwapchainImagesKHR(device, swapchain, &image_count, images.data()),
        "Couldn't get swapchain images!"
    );

    return images;
}
