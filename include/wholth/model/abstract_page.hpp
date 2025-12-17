#ifndef WHOLTH_MODEL_ABSTRACT_PAGE_H_
#define WHOLTH_MODEL_ABSTRACT_PAGE_H_

#include "wholth/buffer_view.hpp"
#include "wholth/pagination.hpp"
#include "wholth/swappable.hpp"
#include <concepts>
#include <system_error>
#include <type_traits>
#include <vector>

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

template <typename T>
concept has_container = requires(T t) {
    t.container;
};

} // namespace wholth::concepts

namespace wholth::model
{

// template <typename Model>
// auto check(const Model&) -> std::error_code;

// todo
// - remove Size
// - move to other namespace
// template <typename T, size_t Size = 20>
template <typename T>
struct SwappableBufferViewsAwareContainer
{
    typedef T value_t;
    // static constexpr size_t size = Size;

    constexpr size_t size() const
    {
        return swappable_buffer_views.view_current().view.size();
    }

    SwappableBufferViewsAwareContainer(){
        // static_assert(Size < 1024, "BE REASONABLE!");
        //
        // std::vector<T> v1{};
        // std::vector<T> v2{};
        //
        // v1.resize(Size);
        // v2.resize(Size);
        // swappable_buffer_views = {
        //     {BufferView<decltype(v1)>{v1}, BufferView<decltype(v2)>{v2}}};
    };

    SwappableBufferViewsAwareContainer(size_t _size)
    {
        assert("BE REASONABLE!" && _size < 1024);

        std::vector<T> v1{};
        std::vector<T> v2{};

        v1.resize(_size);
        v2.resize(_size);
        swappable_buffer_views = {
            {BufferView<decltype(v1)>{v1}, BufferView<decltype(v2)>{v2}}};
    };

    // auto resize(size_t _size) -> void
    // {
    //     swappable_buffer_views.current().view.resize(_size);
    //     swappable_buffer_views.next().view.resize(_size);
    // }

    wholth::Swappable<wholth::BufferView<std::vector<T>>>
        swappable_buffer_views;
};

} // namespace wholth::model

#endif // WHOLTH_MODEL_ABSTRACT_PAGE_H_
