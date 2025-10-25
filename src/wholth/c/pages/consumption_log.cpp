#include "wholth/pages/consumption_log.hpp"
#include "wholth/c/forward.h"
#include "wholth/c/pages/consumption_log.h"
#include "sqlw/statement.hpp"
#include "utils/datetime.hpp"
#include "wholth/entity_manager/consumption_log.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/length_container.hpp"
#include "wholth/utils/to_string_view.hpp"

constexpr auto field_count = 5;
using ::utils::datetime::is_valid_sqlite_datetime;
using wholth::entity_manager::consumption_log::Code;
using wholth::pages::internal::PageType;

static wholth_Error check_from(std::string_view _from)
{
    if (!is_valid_sqlite_datetime(_from))
    {
        static std::string_view msg = "Invalid \"from\" date!";
        return {
            .code = static_cast<int>(Code::CONSUMPTION_LOG_INVALID_DATE),
            .message = {
                .data = msg.data(),
                .size = msg.size(),
            }};
    }

    return wholth_Error_OK;
}

static wholth_Error check_to(std::string_view _to)
{
    if (!is_valid_sqlite_datetime(_to))
    {
        static std::string_view msg = "Invalid \"to\" date!";
        return {
            .code = static_cast<int>(Code::CONSUMPTION_LOG_INVALID_DATE),
            .message = {
                .data = msg.data(),
                .size = msg.size(),
            }};
    }

    return wholth_Error_OK;
}

auto wholth::pages::prepare_consumption_log_stmt(
    sqlw::Statement& stmt,
    const ConsumptionLogQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    using wholth::entity::LengthContainer;
    using wholth::utils::ok;

    if (0 != check_from(query.created_from).code ||
        0 != check_to(query.created_to).code)
    {
        return {LengthContainer{0}, Code::CONSUMPTION_LOG_INVALID_DATE};
    }

    constexpr std::string_view sql = R"sql(
    WITH the_list as (
        SELECT
            cl.id,
            cl.food_id,
            cl.mass,
            cl.consumed_at,
            COALESCE(fl.title, '[N/A]') AS food_title
        FROM consumption_log cl
        LEFT JOIN food_localisation fl
            ON fl.food_id = cl.food_id
                AND fl.locale_id = (SELECT value FROM app_info WHERE field = 'default_locale_id')
        WHERE cl.consumed_at BETWEEN ?1 AND ?2
        ORDER BY cl.consumed_at ASC
    )
    SELECT COUNT(the_list.id), NULL, NULL, NULL, NULL FROM the_list
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT ?3 OFFSET ?4)
    )sql";

    ok(stmt.prepare(sql)) &&
        ok(stmt.bind(1, query.created_from, sqlw::Type::SQL_TEXT)) &&
        ok(stmt.bind(2, query.created_to, sqlw::Type::SQL_TEXT)) &&
        ok(stmt.bind(3, static_cast<int>(pagination.per_page()))) &&
        ok(stmt.bind(
            4,
            static_cast<int>(
                pagination.per_page() * pagination.current_page())));

    return {
        LengthContainer{pagination.per_page() * field_count}, stmt.status()};
}

auto wholth::pages::hydrate(
    ConsumptionLog& data,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void
{
    using wholth::utils::extract;
    auto& buffer = data.container.swappable_buffer_views.next().buffer;
    auto& vec = data.container.swappable_buffer_views.next().view;

    assert(vec.size() > index);

    extract(vec[index].id, lc, buffer);
    extract(vec[index].food_id, lc, buffer);
    extract(vec[index].mass, lc, buffer);
    extract(vec[index].consumed_at, lc, buffer);
    extract(vec[index].food_title, lc, buffer);
}

static bool check_page(const wholth_Page* const page)
{
    return nullptr != page && PageType::CONSUMPTION_LOG == page->data.index();
}

wholth_Page* wholth_pages_consumption_log(uint64_t per_page, bool reset = false)
{
    static std::unique_ptr<wholth_Page> ptr;

    if (nullptr == ptr.get() || reset)
    {
        ptr = std::make_unique<wholth_Page>(
            per_page,
            wholth::pages::ConsumptionLog{
                .query = {}, .container = {per_page}});
    }

    return ptr.get();
}

const wholth_ConsumptionLogArray wholth_pages_consumption_log_array(
    const wholth_Page* const page)
{
    if (!check_page(page) || page->pagination.span_size() == 0)
    {
        return {nullptr, 0};
    }

    const auto& vector = std::get<PageType::CONSUMPTION_LOG>(page->data)
                             .container.swappable_buffer_views.view_current()
                             .view;

    assertm(
        vector.size() >= page->pagination.span_size(),
        "You done goofed here wholth_pages_consumption_log_array) [1]!");

    return {vector.data(), page->pagination.span_size()};
}

wholth_Error wholth_pages_consumption_log_period(
    wholth_Page* const page,
    wholth_StringView from,
    wholth_StringView to)
{
    if (!check_page(page))
    {
        return wholth_Error_OK;
    }

    const auto _from = wholth::utils::to_string_view(from);
    const auto _to = wholth::utils::to_string_view(to);

    auto err = check_from(_from);

    if (err.code != 0)
    {
        return err;
    }

    err = check_to(_to);

    if (err.code != 0)
    {
        return err;
    }

    std::get<PageType::CONSUMPTION_LOG>(page->data).query.created_from = _from;
    std::get<PageType::CONSUMPTION_LOG>(page->data).query.created_to = _to;

    return err;
}
