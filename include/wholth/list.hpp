#ifndef WHOLTH_LIST
#define WHOLTH_LIST

#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "wholth/hydrate.hpp"
#include "wholth/status.hpp"
#include <charconv>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace wholth
{
template <wholth::concepts::is_query Q>
auto check_query(const Q&) -> std::error_code;

template <wholth::concepts::is_query Q>
auto prepare_fill_span_statement(
    const Q& query,
    size_t span_size,
    sqlw::Connection& db_con) -> sqlw::Statement;


template <wholth::concepts::is_query Q>
std::error_code check_numeric_limits(const Q& query, size_t span_size)
{
    using SC = wholth::status::Code;

    if (span_size > std::numeric_limits<int>::max()) {
        return SC::SPAN_SIZE_TOO_BIG;
    }

    if (query.page > std::numeric_limits<int>::max()) {
        return SC::QUERY_PAGE_TOO_BIG;
    }

    const auto offset = span_size * query.page;
    if (offset > std::numeric_limits<int>::max() || (query.page > 0 && offset < span_size)) {
        return SC::QUERY_OFFSET_TOO_BIG;
    }

    return SC::OK;
}

template <typename T, wholth::concepts::is_query Q>
auto fill_span(
    std::span<T> span,
    std::string& buffer,
    uint64_t& count,
    const Q& query,
    sqlw::Connection& con) -> std::error_code
{
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
} // namespace wholth

#endif // WHOLTH_LIST
