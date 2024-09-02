#include "GLFW/glfw3.h"
#include "fmt/color.h"
#include "ui/internal/platform.hpp"
#include "ui/internal/glfw_platform.hpp"

static void glfw_error_callback(int error_code, const char* description)
{
    fmt::print(
        fmt::fg(fmt::color::red),
        "GLFW error [{}]: {}\n",
        error_code,
        description
    );
}

void ui::internal::GlfwPlatform::handle_events()
{
    glfwPollEvents();
}

template<>
ui::internal::PlatformFactory<ui::internal::GlfwPlatform>::PlatformFactory()
{
    glfwSetErrorCallback(glfw_error_callback);
}

template<>
ui::internal::PlatformFactory<ui::internal::GlfwPlatform>::~PlatformFactory()
{
}
