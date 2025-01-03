#ifndef UI_PLATFORM_H_
#define UI_PLATFORM_H_

#include "ui/internal/glfw_imgui.hpp"
#include "ui/internal/glfw_window.hpp"
#include "ui/internal/imgui.hpp"
#include "ui/internal/platform.hpp"

/* #ifdef BUILD_FOR_IOS */
/* #else */

#include "ui/internal/glfw_platform.hpp"
#include "ui/internal/window.hpp"

namespace ui
{
    typedef ui::internal::PlatformFactory<ui::internal::GlfwPlatform> PlatformFactory;
    typedef ui::internal::WindowFactory<ui::internal::GlfwWindow> WindowFactory;
    typedef ui::internal::GlfwImgui ImguiImplementation;
}

/* #endif */

namespace ui
{
    typedef ui::internal::WindowOptions WindowOptions;
    typedef ui::internal::Imgui<ImguiImplementation> Imgui;
}

#endif // UI_PLATFORM_H_
