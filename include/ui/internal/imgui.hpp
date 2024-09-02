#ifndef UI_INTERNAL_IMGUI_H_
#define UI_INTERNAL_IMGUI_H_

#include "vk/device.hpp"
#include "vk/instance.hpp"
#include "vulkan/vulkan_core.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "vk/vk.hpp"

// @todo move elsewhere.
void check_no_msg(VkResult result);

namespace ui::internal
{
    struct ImguiOptions
    {
        const vk::Instance& instance;
        VkPhysicalDevice physical_device;
        const vk::Device& device;
        vk::device::QueueFamily& queue_family;
        VkQueue queue;
        const VkSurfaceCapabilitiesKHR& surface_capabilities;
    };

    template<typename T>
    concept is_imgui_implementation_detail = requires(
        T t
    )
    {
        { t.init() };
        { t.new_frame() };
        { t.shutdown() };
    };

    template <is_imgui_implementation_detail T>
    class Imgui
    {
    public:
        typedef T implementation_detail_t;

        Imgui(const ImguiOptions&, T);
        ~Imgui();

        auto new_frame(float width, float height) -> void;


    private:
        T m_impl;
    };

    template <is_imgui_implementation_detail T>
    Imgui<T>::Imgui(const ImguiOptions& options, T impl) :
        m_impl(impl)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        /* ImGuiIO& io = ImGui::GetIO(); */
        /* (void)io; */

        /* io.DisplaySize.x = 1280;             // set the current display width */
        /* io.DisplaySize.y = 720;             // set the current display height here */

        ImGui::StyleColorsDark();

        this->m_impl.init();

        /* ImGui_ImplGlfw_InstallCallbacks(window.m_glfw_window); */
        ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = options.instance.handle, // *
            .PhysicalDevice = options.physical_device, // *
            .Device = options.device.handle, // *
            .QueueFamily = options.queue_family.index,
            .Queue = options.queue, // *
            .PipelineCache = options.device.pipeline_cache,
            .DescriptorPool = options.device.descriptor_pool, // *
            .Subpass = 0,
            .MinImageCount = options.surface_capabilities.minImageCount, // *
            .ImageCount = static_cast<uint32_t>(options.device.images.size()), // *
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .Allocator = vk::get_allocators(),
            .CheckVkResultFn = check_no_msg,
        };
        ImGui_ImplVulkan_Init(&init_info, options.device.render_pass);
    }

    template <is_imgui_implementation_detail T>
    Imgui<T>::~Imgui()
    {
        this->m_impl.shutdown();

        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();
    }


    template <is_imgui_implementation_detail T>
    void Imgui<T>::new_frame(float width, float height)
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGui_ImplVulkan_NewFrame();

        this->m_impl.new_frame();

        io.DisplaySize.x = width;             // set the current display width
        io.DisplaySize.y = height;             // set the current display height here
        ImGui::NewFrame();
    }
}

#endif // UI_INTERNAL_IMGUI_H_
