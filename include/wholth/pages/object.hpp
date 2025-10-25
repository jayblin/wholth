#ifndef WHOLTH_PAGES_OBJECT_H_
#define WHOLTH_PAGES_OBJECT_H_

#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/swappable.hpp"
#include <cstdint>

namespace wholth::pages
{

// todo rename to fill_object_prepare_statement
template <typename Q>
auto prepare_fill_object_statement(const Q& query, sqlw::Connection& db_con)
    -> sqlw::Statement;

template <typename T, typename Q>
auto fill_object(
    T object,
    std::string& buffer,
    uint64_t& count,
    const Q& query,
    sqlw::Connection& con) -> std::error_code
{
    count = 0;

    auto ec = check_query(query);

    if (sqlw::status::Condition::OK != ec)
    {
        return ec;
    }

    wholth::utils::LengthContainer lc{(wholth::count_fields<T>())};

    std::stringstream buffer_stream;

    auto stmt = prepare_fill_object_statement(query, con);

    if (wholth::status::Condition::OK != stmt.status())
    {
        return stmt.status();
    }

    uint64_t i = 0;
    ec = stmt([&buffer_stream, &lc, &i, &count](sqlw::Statement::ExecArgs e) {
        i++;

        if (i <= wholth::count_fields<T>())
        {
            return;
        }

        buffer_stream << e.column_value;

        lc.add(e.column_value.size());
    });

    if (sqlw::status::Condition::OK != ec)
    {
        return ec;
    }

    if (buffer_stream.rdbuf()->in_avail() <= 0)
    {
        return wholth::pages::Code::NOT_FOUND;
    }

    buffer = buffer_stream.str();

    count = 1;
    object = hydrate<T>(buffer, lc);

    return wholth::pages::Code::OK;
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

} // namespace wholth::controller

#endif // WHOLTH_PAGES_OBJECT_H_
