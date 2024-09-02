#ifndef UI_INTERNAL_WINDOW_H_
#define UI_INTERNAL_WINDOW_H_

#include "vulkan/vulkan_core.h"
#include <concepts>

namespace ui::internal
{
    struct WindowOptions
    {
        float width;
        float height;
    };

    template<typename T>
    concept is_window = requires(
        T t,
        const WindowOptions& options
    )
    {
        { t.should_close() } -> std::same_as<bool>;
        { T(options) };
    };

    template <typename T>
        requires(is_window<T>)
    class WindowFactory
    {
    public:
        WindowFactory();
        ~WindowFactory();

        auto create(const WindowOptions& options) -> T
        {
            return T(options);
        }
    };

    template <typename T>
        requires(is_window<T>)
    class SurfaceFactory
    {
    public:
        /* SurfaceFactory(); */
        /* ~SurfaceFactory(); */

        auto create(VkInstance instance, T& window) -> VkSurfaceKHR;
    };
}

#endif // UI_INTERNAL_WINDOW_H_
