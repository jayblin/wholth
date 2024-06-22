#include "vk/device.hpp"
#include "fmt/color.h"
#include "vk/descriptor_pool.hpp"
#include "vk/render_pass.hpp"
#include "vk/swapchain.hpp"
#include "vk/vk.hpp"
#include <vector>

vk::Device::Device() {}

void vk::Device::init(
    const VkPhysicalDevice physical_device,
    const vk::device::QueueFamily& queue_family,
    const VkSurfaceKHR surface,
    const VkSurfaceCapabilitiesKHR& surface_capabilities
)
{
    if (VK_NULL_HANDLE != this->handle) {
        return;
    }

    const std::vector<const char*> device_extensions {
        "VK_KHR_portability_subset",
        "VK_KHR_swapchain",
    };

    float queue_prioriries[] { 1.0f };
    VkDeviceQueueCreateInfo device_queue_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = queue_family.index,
        .queueCount = 1,
        .pQueuePriorities = queue_prioriries,
    };

    VkDeviceCreateInfo device_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr, // Can connect logigal device to nmultiple physical devices with this. As well as other things.
        /* .flags = , */
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        /* .enabledLayerCount = , // deprecated */
        /* .ppEnabledLayerNames = , // deprecated */
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures = nullptr,
    };

    vk::check(
        vkCreateDevice(
            physical_device,
            &device_create_info,
            vk::get_allocators(),
            &this->handle
        ),
        "Couldn't create device!"
    );

    /* VkQueue queue; */
    /* vkGetDeviceQueue(this->handle, queue_family.index, 0, &queue); */

    uint32_t surface_format_count = 0;
    vk::check(
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &surface_format_count,
            nullptr
        ),
        "Couldn't get surface format count!"
    );

    std::vector<VkSurfaceFormatKHR> surface_formats (surface_format_count);
    vk::check(
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &surface_format_count,
            surface_formats.data()
        ),
        "Couldn't get surface formats!"
    );

    auto surface_format = surface_formats.at(0);

    init_swapchain(surface, surface_format, surface_capabilities);
    init_images();
    init_render_pass(surface_format);
    init_descriptor_pool();
    init_image_acquired_semaphores();
    init_render_complete_semaphores(); 
    init_fences();
    init_command_pools(queue_family);
    init_command_buffers();
    init_image_views(surface_format);
    init_framebuffers(surface_capabilities);
}

vk::Device& vk::Device::operator=(vk::Device&& other)
{
    if (&other != this) {
        this->handle = other.handle;
        this->swapchain = other.swapchain;
        this->render_pass = other.render_pass;
        this->pipeline_cache = other.pipeline_cache;
        this->descriptor_pool = other.descriptor_pool;

        other.handle = VK_NULL_HANDLE;
        other.swapchain = VK_NULL_HANDLE;
        other.render_pass = VK_NULL_HANDLE;
        other.pipeline_cache = VK_NULL_HANDLE;
        other.descriptor_pool = VK_NULL_HANDLE;

        this->images = std::move(other.images);
        this->image_acquired_semaphores = std::move(other.image_acquired_semaphores);
        this->render_complete_semaphores = std::move(other.render_complete_semaphores);
        this->fences = std::move(other.fences);
        this->command_pools = std::move(other.command_pools);
        this->command_buffers = std::move(other.command_buffers);
        this->image_views = std::move(other.image_views);
        this->framebuffers = std::move(other.framebuffers);
    }

    return *this;
}

vk::Device::~Device()
{
    for (size_t i = 0; i < image_acquired_semaphores.size(); i++)
    {
        vkDestroySemaphore(this->handle, image_acquired_semaphores[i], vk::get_allocators());
    }

    for (size_t i = 0; i < render_complete_semaphores.size(); i++)
    {
        vkDestroySemaphore(this->handle, render_complete_semaphores[i], vk::get_allocators());
    }

    for (size_t i = 0; i < fences.size(); i++)
    {
        vkDestroyFence(this->handle, fences[i], vk::get_allocators());
    }

    for (size_t i = 0; i < image_views.size(); i++)
    {
        vkDestroyImageView(this->handle, image_views[i], vk::get_allocators());
    }

    for (size_t i = 0; i < command_pools.size(); i++)
    {
        vkFreeCommandBuffers(this->handle, command_pools[i], 1, &command_buffers[i]);
        vkDestroyCommandPool(this->handle, command_pools[i], vk::get_allocators());
    }

    for (size_t i = 0; i < framebuffers.size(); i++)
    {
        vkDestroyFramebuffer(this->handle, framebuffers[i], vk::get_allocators());
    }

    vkDestroyDescriptorPool(this->handle, descriptor_pool, vk::get_allocators());
    vkDestroyPipelineCache(this->handle, pipeline_cache, vk::get_allocators());
    vkDestroyRenderPass(this->handle, render_pass, vk::get_allocators());
    vkDestroySwapchainKHR(this->handle, swapchain, vk::get_allocators());
    vkDestroyDevice(this->handle, vk::get_allocators());
}

VkSwapchainKHR vk::Device::init_swapchain(
    VkSurfaceKHR surface,
    const VkSurfaceFormatKHR& surface_format,
    const VkSurfaceCapabilitiesKHR& surface_capabilities
)
{
    if (this->swapchain != VK_NULL_HANDLE) {
        return this->swapchain;
    }

    vk::swapchain::create(
        &this->swapchain,
        this->handle,
        surface,
        surface_format,
        surface_capabilities
    );

    return this->swapchain;
}

VkRenderPass vk::Device::init_render_pass(
    const VkSurfaceFormatKHR& surface_format
)
{
    if (this->render_pass != VK_NULL_HANDLE) {
        return this->render_pass;
    }

    vk::render_pass::create(
        &this->render_pass,
        this->handle,
        surface_format
    );

    return this->render_pass;
}

VkDescriptorPool vk::Device::init_descriptor_pool()
{
    if (this->descriptor_pool != VK_NULL_HANDLE) {
        return this->descriptor_pool;
    }

    vk::descriptor_pool::create(
        &this->descriptor_pool,
        this->handle
    );

    return this->descriptor_pool;
}

std::span<VkImage> vk::Device::init_images()
{
    if (this->images.size() > 0) {
        return this->images;
    }

    this->images = vk::swapchain::get_images(
        this->handle,
        this->swapchain
    );

    return this->images;
}

std::span<VkSemaphore> vk::Device::init_image_acquired_semaphores()
{
    if (this->image_acquired_semaphores.size() > 0) {
        return this->image_acquired_semaphores;
    }

    image_acquired_semaphores.resize(this->images.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < this->image_acquired_semaphores.size(); i++)
    {
        VkSemaphoreCreateInfo semaphore_create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };
        vk::check(
            vkCreateSemaphore(this->handle, &semaphore_create_info, vk::get_allocators(), &this->image_acquired_semaphores[i]),
            "Couldn't create semaphore"
        );
    }

    return this->image_acquired_semaphores;
}

std::span<VkSemaphore> vk::Device::init_render_complete_semaphores()
{
    if (this->render_complete_semaphores.size() > 0) {
        return this->render_complete_semaphores;
    }

    render_complete_semaphores.resize(this->images.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < this->render_complete_semaphores.size(); i++)
    {
        VkSemaphoreCreateInfo semaphore_create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };
        vk::check(
            vkCreateSemaphore(this->handle, &semaphore_create_info, vk::get_allocators(), &this->render_complete_semaphores[i]),
            "Couldn't create semaphore"
        );
    }

    return this->render_complete_semaphores;
}

std::span<VkFence> vk::Device::init_fences()
{
    if (this->fences.size() > 0) {
        return this->fences;
    }

    fences.resize(this->images.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < fences.size(); i++)
    {
        VkFenceCreateInfo semaphore_create_info {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        vk::check(
            vkCreateFence(this->handle, &semaphore_create_info, vk::get_allocators(), &this->fences[i]),
            "Couldn't create fence"
        );
    }

    return this->fences;
}

/* std::span<VkCommandPool> vk::Device::init_command_pools(const vk::device::QueueFamilies& queue_families) */
std::span<VkCommandPool> vk::Device::init_command_pools(const vk::device::QueueFamily& queue_family)
{
    if (this->command_pools.size() > 0) {
        return this->command_pools;
    }

    this->command_pools.resize(this->images.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < this->command_pools.size(); i++)
    {
        // @todo: research this
        VkCommandPoolCreateInfo command_pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            /* .queueFamilyIndex = queue_families.graphics.index, */
            .queueFamilyIndex = queue_family.index,
        };
        vk::check(
            vkCreateCommandPool(this->handle, &command_pool_create_info, vk::get_allocators(), &this->command_pools[i]),
            "Couldn't create command pool"
        );
    }

    return this->command_pools;
}

std::span<VkCommandBuffer> vk::Device::init_command_buffers()
{
    if (this->command_buffers.size() > 0) {
        return this->command_buffers;
    }

    this->command_buffers.resize(this->images.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < this->command_buffers.size(); i++)
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pools[i],
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        vk::check(
            vkAllocateCommandBuffers(this->handle, &command_buffer_allocate_info, &this->command_buffers[i]),
            "Couldn't allocate command buffers"
        );
    }

    return this->command_buffers;
}

std::span<VkImageView> vk::Device::init_image_views(const VkSurfaceFormatKHR& surface_format)
{
    if (this->image_views.size() > 0) {
        return this->image_views;
    }

    image_views.resize(this->images.size(), VK_NULL_HANDLE);
    VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    VkComponentMapping components = {
        .r = VK_COMPONENT_SWIZZLE_R,
        .g = VK_COMPONENT_SWIZZLE_G,
        .b = VK_COMPONENT_SWIZZLE_B,
        .a = VK_COMPONENT_SWIZZLE_A,
    };

    for (size_t i = 0; i < this->image_views.size(); i++)
    {
        VkImageViewCreateInfo image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surface_format.format,
            .components = components,
            .subresourceRange = image_range,
        };
        vk::check(
            vkCreateImageView(this->handle, &image_view_create_info, vk::get_allocators(), &this->image_views[i]),
            "Couldn't create image view"
        );
    }

    return this->image_views;
}

std::span<VkFramebuffer> vk::Device::init_framebuffers(const VkSurfaceCapabilitiesKHR& surface_capabilities)
{
    if (this->framebuffers.size() > 0) {
        return this->framebuffers;
    }

    framebuffers.resize(this->images.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i < framebuffers.size(); i++)
    {
        VkImageView attm[1];
        attm[0] = this->image_views[i];
        VkFramebufferCreateInfo framebuffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render_pass,
            .attachmentCount = 1,
            .pAttachments = attm,
            .width = surface_capabilities.currentExtent.width,
            .height = surface_capabilities.currentExtent.height,
            .layers = 1,
        };

        vk::check(
            vkCreateFramebuffer(this->handle, &framebuffer_create_info, vk::get_allocators(), &this->framebuffers[i]),
            "Couldn't create framebuffer!"
        );
    }

    return this->framebuffers;
}
