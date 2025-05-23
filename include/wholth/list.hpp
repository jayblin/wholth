#ifndef WHOLTH_LIST
#define WHOLTH_LIST

#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/utils.hpp"
#include "wholth/context.hpp"
#include "wholth/hydrate.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/status.hpp"
#include <charconv>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace wholth
{
/* template <wholth::concepts::is_queryable Q> */
/* auto check_query(const Q&) -> std::error_code; */
template <typename Q>
auto check_query(const Q&) -> std::error_code;

// todo move elsewhere?
template <wholth::concepts::is_context_aware Q>
auto check_query(const Q& query) -> std::error_code
{
    if (query.ctx.locale_id.size() == 0 ||
        !sqlw::utils::is_numeric(query.ctx.locale_id))
    {
        return wholth::status::Code::INVALID_LOCALE_ID;
    }

    return wholth::status::Code::OK;
}

/* template <wholth::concepts::is_queryable Q> */
template <typename Q>
auto prepare_fill_span_statement(
    const Q& query,
    size_t span_size,
    sqlw::Connection& db_con) -> sqlw::Statement;

template <wholth::concepts::has_pagination Q>
std::error_code check_numeric_limits(const Q& query, size_t span_size)
{
    using SC = wholth::status::Code;

    if (span_size > std::numeric_limits<int>::max())
    {
        return SC::SPAN_SIZE_TOO_BIG;
    }

    if (query.pagination.current_page() > std::numeric_limits<int>::max())
    {
        return SC::QUERY_PAGE_TOO_BIG;
    }

    const auto offset = span_size * query.pagination.current_page();
    if (offset > std::numeric_limits<int>::max() ||
        (query.pagination.current_page() > 0 && offset < span_size))
    {
        return SC::QUERY_OFFSET_TOO_BIG;
    }

    return SC::OK;
}

template <typename T, wholth::concepts::has_pagination Q>
auto fill_span(
    std::span<T> span,
    std::string& buffer,
    uint64_t& count,
    const Q& query,
    sqlw::Connection& con) -> std::error_code
{
    if (span.size() != query.pagination.per_page())
    {
        // todo change message
        throw std::runtime_error("AYYYYYYYY LMAO");
    }

    count = 0;

    auto ec = check_numeric_limits(query, span.size());

    if (wholth::status::Condition::OK != ec)
    {
        return ec;
    }

    ec = check_query(query);

    if (wholth::status::Condition::OK != ec)
    {
        return ec;
    }

    wholth::utils::LengthContainer lc{
        (span.size() * wholth::count_fields<T>())};

    std::stringstream buffer_stream;

    auto stmt = prepare_fill_span_statement(query, span.size(), con);

    if (wholth::status::Condition::OK != stmt.status())
    {
        return stmt.status();
    }

    uint64_t i = 0;
    ec = stmt([&buffer_stream, &lc, &i, &count](sqlw::Statement::ExecArgs e) {
        i++;

        if (1 == i)
        {
            auto res = std::from_chars(
                e.column_value.data(),
                e.column_value.data() + e.column_value.size(),
                count);
            if (res.ec != std::errc())
            {
                // todo ????
                count = 0;
            }
        }

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
        return wholth::status::Code::ENTITY_NOT_FOUND;
    }

    buffer = buffer_stream.str();

    for (size_t j = 0; j < span.size(); j++)
    {
        span[j] = hydrate<T>(buffer, lc);
    }

    return wholth::status::Code::OK;
}

template <typename Q>
auto prepare_fill_object_statement(
    const Q& query,
    sqlw::Connection& db_con) -> sqlw::Statement;

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

    if (wholth::status::Condition::OK != ec)
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
        return wholth::status::Code::ENTITY_NOT_FOUND;
    }

    buffer = buffer_stream.str();

    count = 1;
    object = hydrate<T>(buffer, lc);

    return wholth::status::Code::OK;
}


} // namespace wholth

#endif // WHOLTH_LIST
