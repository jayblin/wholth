#include "ui/imgui.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "vk/instance.hpp"
#include "vk/vk.hpp"

static void check_no_msg(VkResult result)
{
    vk::check(result, "[No message]");
}

ui::Imgui::Imgui(
    const vk::Instance& instance,
    VkPhysicalDevice physical_device,
    const vk::Device& device,
    vk::device::QueueFamily& queue_family,
    VkQueue queue,
    const VkSurfaceCapabilitiesKHR& surface_capabilities,
    ui::GlfwWindow& window
)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    /* ImGuiIO& io = ImGui::GetIO(); */
    /* (void)io; */

    /* io.DisplaySize.x = 1280;             // set the current display width */
    /* io.DisplaySize.y = 720;             // set the current display height here */

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window.handle, false);
    /* ImGui_ImplGlfw_InstallCallbacks(window.m_glfw_window); */
    ImGui_ImplVulkan_InitInfo init_info = {
        .Instance = instance.handle, // *
        .PhysicalDevice = physical_device, // *
        .Device = device.handle, // *
        .QueueFamily = queue_family.index,
        .Queue = queue, // *
        .PipelineCache = device.pipeline_cache,
        .DescriptorPool = device.descriptor_pool, // *
        .Subpass = 0,
        .MinImageCount = surface_capabilities.minImageCount, // *
        .ImageCount = static_cast<uint32_t>(device.images.size()), // *
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .Allocator = vk::get_allocators(),
        .CheckVkResultFn = check_no_msg,
    };
    ImGui_ImplVulkan_Init(&init_info, device.render_pass);
}

ui::Imgui::~Imgui()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplVulkan_Shutdown();
    ImGui::DestroyContext();
}
