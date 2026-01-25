#include "fmt/core.h"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/pages/nutrient.h"
#include "wholth/c/error.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/pages/nutrient.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/length_container.hpp"
#include "wholth/utils/str_replace.hpp"
#include "wholth/utils/to_error.hpp"
#include "wholth/utils/to_string_view.hpp"
#include "wholth/error.hpp"
#include <memory>
#include <cassert>
#include <sstream>
#include <string_view>

using wholth::pages::internal::PageType;
using wholth::utils::from_error_code;
using wholth::utils::is_valid_id;
using wholth::utils::to_error;
using wholth::utils::to_string_view;

template <>
struct wholth::error::is_error_code_enum<wholth_pages_nutrient_Code>
    : std::true_type
{
};

auto wholth::pages::hydrate(
    wholth::pages::Nutrient& data,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void
{
    auto& buffer = data.container.buffer;
    auto& vec = data.container.view;

    assert(vec.size() > index);

    wholth::utils::extract(vec[index].id, lc, buffer);
    wholth::utils::extract(vec[index].title, lc, buffer);
    wholth::utils::extract(vec[index].unit, lc, buffer);
    wholth::utils::extract(vec[index].position, lc, buffer);
}

auto wholth::pages::prepare_nutrient_stmt(
    sqlw::Statement& stmt,
    const NutrientQuery& model,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    using wholth::entity::LengthContainer;
    using wholth::utils::ok;

    static_assert(
        wholth::error::is_error_code_enum<wholth_pages_nutrient_Code>::value ==
        true);
    if (model.locale_id.empty())
    {
        return {
            LengthContainer{},
            wholth_pages_nutrient_Code::NUTRIENT_PAGE_BAD_LOCALE_ID};
    }

    constexpr std::string_view sql_tpl = R"sql(
    WITH 
    filtered_list AS (
        SELECT
            n.id,
            nl_fts5.title,
            n.unit,
            n.position,
            CASE WHEN nl.locale_id <> ?3
                THEN NULL
                ELSE nl.locale_id
            END AS locale_id
        FROM nutrient n
        INNER JOIN nutrient_localisation nl
            ON nl.nutrient_id = n.id
        INNER JOIN nutrient_localisation_fts5 nl_fts5
            ON nl_fts5.rowid = nl.nl_fts5_rowid 
        WHERE {0} 
    ),
    partitioned_list AS (
        SELECT
            fl.*,
            row_number() OVER (
                PARTITION BY id
                ORDER BY locale_id ASC NULLS LAST
            ) AS rn
        FROM filtered_list AS fl 
    ),
    the_list AS (
         SELECT id, title, unit, position
         FROM partitioned_list
         WHERE rn = 1
         GROUP BY id
         ORDER BY position ASC
    )
    SELECT COUNT(id), NULL, NULL, NULL FROM the_list
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT ?1 OFFSET ?2)
    )sql";

    std::string title_where = "1=1";

    if (!model.title.empty())
    {
        title_where = R"sql(
            n.id IN (
                SELECT
                    nl.nutrient_id
                FROM nutrient_localisation_fts5 nl_fts5
                INNER JOIN nutrient_localisation nl
                    ON nl_fts5.rowid = nl.nl_fts5_rowid 
                WHERE nutrient_localisation_fts5 MATCH ?4 
            )
        )sql";
    }

    const std::string sql = fmt::format(sql_tpl, title_where);

    ok(stmt.prepare(sql))                                            //
        && ok(stmt.bind(1, static_cast<int>(pagination.per_page()))) //
        && ok(stmt.bind(
               2,
               static_cast<int>(
                   pagination.per_page() * pagination.current_page()))) //
        && ok(stmt.bind(3, model.locale_id, sqlw::Type::SQL_INT));

    if (ok(stmt) && !model.title.empty())
    {
        const std::string values =
            "{title}:" + wholth::utils::str_replace(model.title, ",", " OR ");

        stmt.bind(4, values, sqlw::Type::SQL_TEXT);
    }

    return {LengthContainer{pagination.per_page() * 4}, stmt.status()};
}

extern "C" wholth_Error wholth_pages_nutrient(
    wholth_Page** page,
    uint64_t per_page)
{
    auto err = wholth_pages_new(page);

    if (!wholth_error_ok(&err))
    {
        return err;
    }

    wholth::pages::Nutrient page_data{.query = {}, .container = {}};
    page_data.container.view.resize(per_page);

    **page = {per_page, page_data};

    return wholth_Error_OK;
}

static bool check_page(const wholth_Page* const page)
{
    return nullptr != page &&
           wholth::pages::internal::PageType::NUTRIENT == page->data.index();
}

extern "C" const wholth_NutrientArray wholth_pages_nutrient_array(
    const wholth_Page* const page)
{
    if (!check_page(page) || page->pagination.span_size() == 0)
    {
        return {nullptr, 0};
    }

    const auto& vector =
        std::get<PageType::NUTRIENT>(page->data).container.view;

    assertm(
        vector.size() >= page->pagination.span_size(),
        "You done goofed here wholth_pages_nutrient() [1]!");

    return {vector.data(), page->pagination.span_size()};
}

enum ModelField : int
{
    TITLE,
    LOCALE_ID,
};

static wholth_Error set_model_field(
    wholth_Page* const page,
    ModelField field,
    wholth_StringView value)
{
    if (!check_page(page))
    {
        return from_error_code(NUTRIENT_PAGE_TYPE_MISMATCH);
    }

    auto& _page =
        std::get<wholth::pages::internal::PageType::NUTRIENT>(page->data);

    std::error_code ec{};
    switch (field)
    {
    case TITLE:
        _page.query.title = to_string_view(value);
        break;
    case LOCALE_ID:
        const auto id = to_string_view(value);
        if (!is_valid_id(id))
        {
            ec = NUTRIENT_PAGE_BAD_LOCALE_ID;
        }
        else
        {
            _page.query.locale_id = id;
        }
        break;
    }

    return ec ? from_error_code(ec) : wholth_Error_OK;
}

extern "C" wholth_Error wholth_pages_nutrient_title(
    wholth_Page* const page,
    wholth_StringView title)
{
    return set_model_field(page, ModelField::TITLE, title);
}

extern "C" wholth_Error wholth_pages_nutrient_locale_id(
    wholth_Page* const page,
    wholth_StringView locale_id)
{
    return set_model_field(page, ModelField::LOCALE_ID, locale_id);
}
