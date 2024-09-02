#ifndef UI_INTERNAL_PLATFORM_H_
#define UI_INTERNAL_PLATFORM_H_

#include <concepts>

namespace ui::internal
{
    template<typename T>
    concept is_platform = requires(
        T t
    )
    {
        { t.handle_events() } -> std::same_as<void>;
    };

    template <typename T>
        requires(is_platform<T>)
    class PlatformFactory
    {
    public:
        PlatformFactory();
        ~PlatformFactory();

        auto create() -> T
        {
            return T {};
        }
    };
}

#endif // UI_INTERNAL_PLATFORM_H_
