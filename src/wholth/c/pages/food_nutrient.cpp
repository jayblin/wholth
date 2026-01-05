#include "wholth/entity/nutrient.hpp"
#include "fmt/core.h"
#include "sqlw/statement.hpp"
#include "wholth/c/entity/nutrient.h"
#include "wholth/c/pages/food_nutrient.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/entity_manager/food.hpp"
#include "wholth/error.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/pages/food_nutrient.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/length_container.hpp"
#include "wholth/utils/to_error.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <memory>
#include <cassert>

using wholth::utils::from_error_code;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;

template <>
struct wholth::error::is_error_code_enum<wholth_pages_food_nutrient_Code>
    : std::true_type
{
};

static_assert(wholth::entity::is_nutrient<wholth_Nutrient>);
static_assert(wholth::entity::count_fields<wholth_Nutrient>() == 5);

// template <wholth::entity::is_nutrient T>
auto wholth::pages::hydrate( // const std::string& buffer,
    FoodNutrient& data,
    size_t index,
    wholth::entity::LengthContainer& lc)
    // -> T
    -> void
{
    auto& buffer = data.container.buffer;
    auto& vec = data.container.view;

    assert(vec.size() > index);

    wholth::utils::extract(vec[index].id, lc, buffer);
    wholth::utils::extract(vec[index].title, lc, buffer);
    wholth::utils::extract(vec[index].value, lc, buffer);
    wholth::utils::extract(vec[index].unit, lc, buffer);
    wholth::utils::extract(vec[index].position, lc, buffer);
}

auto wholth::pages::prepare_food_nutrient_stmt(
    sqlw::Statement& stmt,
    const FoodNutrientQuery& model,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    using wholth::entity::LengthContainer;
    using wholth::utils::ok;

    if (!is_valid_id(model.food_id))
    {
        return {LengthContainer{0}, FOOD_NUTRIENT_PAGE_BAD_FOOD_ID};
    }

    if (!is_valid_id(model.locale_id))
    {
        return {LengthContainer{0}, FOOD_NUTRIENT_PAGE_BAD_LOCALE_ID};
    }

    // nutrient value in schientific notaion
    // IIF(
    //     fn.value < 0.00000001,
    //     ROUND(fn.value * 100000000, 1) || 'e-8',
    //     IIF(
    //         fn.value < 0.0000001,
    //         ROUND(fn.value * 10000000, 1) || 'e-7',
    //         IIF(
    //             fn.value < 0.000001,
    //             ROUND(fn.value * 1000000, 1) || 'e-6',
    //             IIF(
    //                 fn.value < 0.00001,
    //                 ROUND(fn.value * 100000, 1) || 'e-5',
    //                 IIF(
    //                     fn.value < 0.0001,
    //                     ROUND(fn.value * 10000, 1) || 'e-4',
    //                     IIF(
    //                         fn.value < 0.001,
    //                         ROUND(fn.value * 1000, 1) || 'e-3',
    //                         IIF(
    //                             fn.value < 0.01,
    //                             ROUND(fn.value * 100, 1) || 'e-2',
    //                             IIF(
    //                                 fn.value < 0.1,
    //                                 ROUND(fn.value * 10, 1) || 'e-1',
    //                                 IIF(
    //                                     fn.value < 10,
    //                                     ROUND(fn.value, 2),
    //                                     ROUND(fn.value, 1)
    //                                 )
    //                             )
    //                         )
    //                     )
    //                 )
    //             )
    //         )
    //     )
    // )
    constexpr std::string_view sql_tpl = R"sql(
    WITH
    text_search AS (
        SELECT
            rowid,
            title
        FROM
        nutrient_localisation_fts5
        WHERE {0}
    ),
    filtered_list AS (
        SELECT
            n.id,
            nl_fts5.title,
            fn.value,
            n.unit,
            n.position,
            CASE WHEN nl.locale_id <> ?4
                THEN NULL
                ELSE nl.locale_id
            END AS locale_id
        FROM nutrient n
        INNER JOIN food_nutrient fn
            ON fn.nutrient_id = n.id
                AND fn.food_id = ?1
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
                ORDER BY locale_id NULLS LAST
            ) AS rn
        FROM filtered_list AS fl 
    ),
    the_list as (
        SELECT
            id,
            title,
            value,
            unit,
            position
        FROM partitioned_list
        WHERE rn = 1
        GROUP BY id
        ORDER BY position ASC
    )
    SELECT COUNT(id), NULL, NULL, NULL, NULL FROM the_list
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT ?2 OFFSET ?3)
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
                WHERE nutrient_localisation_fts5 MATCH ?5 
            )
        )sql";
    }

    const std::string sql = fmt::format(sql_tpl, title_where);

    ok(stmt.prepare(sql))                                            //
        && ok(stmt.bind(1, model.food_id, sqlw::Type::SQL_INT))      //
        && ok(stmt.bind(2, static_cast<int>(pagination.per_page()))) //
        && ok(stmt.bind(
               3,
               static_cast<int>(
                   pagination.per_page() * pagination.current_page()))) //
        && ok(stmt.bind(4, model.locale_id, sqlw::Type::SQL_INT));

    if (ok(stmt) && model.title.size() > 0)
    {
        std::string title_param_value = "{title}:" + model.title;
        stmt.bind(5, title_param_value, sqlw::Type::SQL_TEXT);
    }

    return {
        wholth::entity::LengthContainer{
            pagination.per_page() *
            wholth::entity::count_fields<wholth_Nutrient>()},
        stmt.status()};
}

extern "C" wholth_Error wholth_pages_food_nutrient(
    wholth_Page** page,
    uint64_t per_page)
{
    auto err = wholth_pages_new(page);

    if (!wholth_error_ok(&err))
    {
        return err;
    }

    wholth::pages::FoodNutrient page_data{.query = {}, .container = {}};
    page_data.container.view.resize(per_page);

    **page = {per_page, page_data};

    return wholth_Error_OK;
}

static bool check_page(const wholth_Page* const page)
{
    return nullptr != page &&
           wholth::pages::internal::PageType::FOOD_NUTRIENT ==
               page->data.index();
}

// todo remove
extern "C" const wholth_FoodNutrientArray wholth_pages_food_nutrient_array(
    const wholth_Page* const page)
{
    if (!check_page(page) || page->pagination.span_size() == 0)
    {
        return {nullptr, 0};
    }

    const auto& vector =
        std::get<wholth::pages::internal::PageType::FOOD_NUTRIENT>(page->data)
            .container.view;

    assertm(
        vector.size() >= page->pagination.span_size(),
        "You done goofed here wholth_pages_nutrient() [1]!");

    return {vector.data(), page->pagination.span_size()};
}

enum ModelField : int
{
    FOOD_ID,
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
        return from_error_code(FOOD_NUTRIENT_PAGE_TYPE_MISMATCH);
    }

    auto& _page =
        std::get<wholth::pages::internal::PageType::FOOD_NUTRIENT>(page->data);

    std::error_code ec{};
    switch (field)
    {
    case TITLE:
        _page.query.title = to_string_view(value);
        break;
    case LOCALE_ID: {
        const auto id = to_string_view(value);
        if (!is_valid_id(id))
        {
            ec = FOOD_NUTRIENT_PAGE_BAD_LOCALE_ID;
        }
        else
        {
            _page.query.locale_id = id;
        }
        break;
    }
    case FOOD_ID: {
        const auto id = to_string_view(value);
        if (!is_valid_id(id))
        {
            ec = FOOD_NUTRIENT_PAGE_BAD_FOOD_ID;
        }
        else
        {
            _page.query.food_id = id;
        }
        break;
    }
    }

    return ec ? from_error_code(ec) : wholth_Error_OK;
}

// todo test
extern "C" wholth_Error wholth_pages_food_nutrient_food_id(
    wholth_Page* const page,
    wholth_StringView food_id)
{
    return set_model_field(page, ModelField::FOOD_ID, food_id);
}

// todo test
extern "C" wholth_Error wholth_pages_food_nutrient_title(
    wholth_Page* const page,
    wholth_StringView title)
{
    return set_model_field(page, ModelField::TITLE, title);
}

// todo test
extern "C" wholth_Error wholth_pages_food_nutrient_locale_id(
    wholth_Page* const page,
    wholth_StringView id)
{
    return set_model_field(page, ModelField::LOCALE_ID, id);
}
