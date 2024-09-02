#ifndef UI_INTERNAL_GLFW_IMGUI_H_
#define UI_INTERNAL_GLFW_IMGUI_H_

#include "backends/imgui_impl_glfw.h"
#include "ui/internal/glfw_window.hpp"

namespace ui::internal
{
    struct GlfwImgui
    {
        ui::internal::GlfwWindow& window;

        void init()
        {
            ImGui_ImplGlfw_InitForVulkan(this->window.handle, false);
        }

        void new_frame()
        {
            ImGui_ImplGlfw_NewFrame();
        }

        void shutdown()
        {
            ImGui_ImplGlfw_Shutdown();
        }
    };
}

#endif // UI_INTERNAL_GLFW_IMGUI_H_
