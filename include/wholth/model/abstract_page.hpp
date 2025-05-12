#ifndef WHOLTH_MODEL_ABSTRACT_PAGE_H_
#define WHOLTH_MODEL_ABSTRACT_PAGE_H_

#include "wholth/buffer_view.hpp"
#include "wholth/page.hpp"
#include "wholth/swappable.hpp"
#include <concepts>
#include <type_traits>

namespace wholth::concepts
{
template <typename T>
concept is_fetch_aware = requires(T t) {
    t.is_fetching;
    requires std::same_as<decltype(t.is_fetching), std::atomic<bool>>;
};

template <typename T>
concept has_swappable_buffer_views = requires(T t) {
    t.swappable_buffer_views;
    requires wholth::concepts::is_swappable<decltype(t.swappable_buffer_views)>;
};

template <typename T>
concept has_pagination = requires(T t) {
    t.pagination;
    requires std::same_as<decltype(t.pagination), wholth::Pagination>;
};

} // namespace wholth::concepts

namespace wholth::model
{

template <typename T, size_t Size = 20>
struct SwappableBufferViewsAwareContainer
{
    typedef T value_t;
    static constexpr size_t size = Size;

    SwappableBufferViewsAwareContainer()
    {
        static_assert(Size < 1024, "BE REASONABLE!");

        std::vector<T> v1{};
        std::vector<T> v2{};

        v1.resize(Size);
        v2.resize(Size);
        swappable_buffer_views = {
            {BufferView<decltype(v1)>{v1}, BufferView<decltype(v2)>{v2}}};
    };

    wholth::Swappable<wholth::BufferView<std::vector<T>>>
        swappable_buffer_views;
};

} // namespace wholth::model

#endif // WHOLTH_MODEL_ABSTRACT_PAGE_H_
