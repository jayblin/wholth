#ifndef WHOLTH_CONTROLLER_ABSTRACT_PAGE_H_
#define WHOLTH_CONTROLLER_ABSTRACT_PAGE_H_

#include "wholth/buffer_view.hpp"
#include "wholth/list.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/swappable.hpp"
#include <cstdint>

namespace wholth::controller
{

template <typename T>
    requires wholth::concepts::is_fetch_aware<T> &&
             wholth::concepts::has_pagination<T>
bool advance(T& model, uint64_t by)
{
    return !model.is_fetching && model.pagination.advance(by);
}

template <typename T>
    requires wholth::concepts::is_fetch_aware<T> &&
             wholth::concepts::has_pagination<T>
bool retreat(T& model, uint64_t by)
{
    return !model.is_fetching && model.pagination.retreat(by);
}

template <typename T>
    requires wholth::concepts::is_fetch_aware<T> &&
             wholth::concepts::has_pagination<T>
bool skip_to(T& model, uint64_t page)
{
    return !model.is_fetching && model.pagination.skip_to(page);
}

template <typename Model, typename Container>
    requires wholth::concepts::is_fetch_aware<Model> &&
             wholth::concepts::is_swappable<Container> &&
             wholth::concepts::is_buffer_view<typename Container::value_t>
std::error_code fill_object_through_model(
    Container& container,
    Model& model,
    sqlw::Connection& connection)
{
    if (sqlw::status::Condition::OK != connection.status())
    {
        // todo panic here maybe?
        return connection.status();
    }

    model.is_fetching.store(true, std::memory_order_seq_cst);

    auto& items = container.next().view;
    auto& buffer = container.next().buffer;
    uint64_t new_count = 0;
    // todo about learn that typename thing
    const auto ec = wholth::fill_object<typename Container::value_t::value_t>(
        items, buffer, new_count, model, connection);

    container.swap();

    // @todo learn about memeory order
    model.is_fetching.store(false, std::memory_order_seq_cst);

    return ec;
}

template <typename Model, typename Container>
    requires wholth::concepts::is_fetch_aware<Model> &&
             wholth::concepts::has_pagination<Model> &&
             wholth::concepts::has_swappable_buffer_views<Container>
std::error_code fill_container_through_model(
    Container& container,
    Model& model,
    sqlw::Connection& connection)
{
    if (sqlw::status::Condition::OK != connection.status())
    {
        // todo panic here maybe?
        return connection.status();
    }

    model.is_fetching.store(true, std::memory_order_seq_cst);
    auto& items = container.swappable_buffer_views.next().view;
    auto& buffer = container.swappable_buffer_views.next().buffer;
    uint64_t new_count = 0;
    const auto ec = wholth::fill_span<typename Container::value_t>(
        items, buffer, new_count, model, connection);

    model.pagination.count(new_count);

    model.pagination.update();
    container.swappable_buffer_views.swap();

    for (auto& item : container.swappable_buffer_views.next().view)
    {
        item = {};
    }

    // @todo learn about memeory order
    model.is_fetching.store(false, std::memory_order_seq_cst);

    return ec;
}

} // namespace wholth::controller

#endif // WHOLTH_CONTROLLER_ABSTRACT_PAGE_H_
