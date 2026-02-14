#include "wholth/c/pages/ingredient.h"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/entity/length_container.hpp"
#include "wholth/entity_manager/food.hpp"
#include "wholth/error.hpp"
#include "wholth/pages/ingredient.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/length_container.hpp"
#include "wholth/utils/str_replace.hpp"
#include "wholth/utils/to_error.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <cstddef>
#include <memory>
#include <cassert>
#include <type_traits>

template <>
struct wholth::error::is_error_code_enum<wholth_pages_ingredient_Code>
    : std::true_type
{
};

constexpr auto field_count = 6;
using wholth::pages::internal::PageType;

auto wholth::pages::hydrate(
    Ingredient& data,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void
{
    auto& buffer = data.container.buffer;
    auto& vec = data.container.view;

    assert(vec.size() > index);

    wholth::utils::extract(vec[index].id, lc, buffer);
    wholth::utils::extract(vec[index].food_id, lc, buffer);
    wholth::utils::extract(vec[index].food_title, lc, buffer);
    wholth::utils::extract(vec[index].canonical_mass_g, lc, buffer);
    wholth::utils::extract(vec[index].ingredient_count, lc, buffer);
    wholth::utils::extract(vec[index].ingredients_mass_g, lc, buffer);
}

auto wholth::pages::prepare_ingredient_stmt(
    sqlw::Statement& stmt,
    const IngredientQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    using wholth::entity::LengthContainer;
    using wholth::utils::ok;

    if (!wholth::utils::is_valid_id(query.food_id))
    {
        return {
            LengthContainer{0},
            wholth::entity_manager::food::Code::FOOD_INVALID_ID};
    }

    if (!wholth::utils::is_valid_id(query.locale_id))
    {
        return {LengthContainer{0}, INGREDIENT_PAGE_BAD_LOCALE_ID};
    }

    // std::vector<std::string_view> titles{};
    std::string match_title = "";
    std::string where = "1=1";

    if (!query.title.empty())
    {
        if (query.title.size() < 3)
        {
            return {LengthContainer{0}, INGREDIENT_PAGE_TITLE_TOO_SHORT};
        }

        match_title = fmt::format(
            "{{title}}:({0})",
            wholth::utils::str_replace(query.title, ",", " OR "));
        where = "EXISTS (SELECT food_id FROM text_search WHERE food_id = f.id)";
    }

    constexpr std::string_view sql = R"sql(
    -- PRAGMA integrity_check;
    WITH
    text_search AS NOT MATERIALIZED (
        SELECT
            fl.food_id      AS food_id,
            fl_fts5.title   AS title,
            fl.locale_id    AS locale_id,
            fl_fts5.rowid   AS rowid
        FROM food_localisation_fts5 fl_fts5
        INNER JOIN food_localisation fl
            ON fl_fts5.rowid = fl.fl_fts5_rowid
        WHERE food_localisation_fts5 MATCH ?5
    ),
    filtered_list AS (
        SELECT
            rsf.id AS id,
            f.id AS food_id,
            COALESCE(fl_fts5.title, '[N/A]') AS food_title,
            rsf.canonical_mass AS canonical_mass_g,
            (
                SELECT
                    COUNT(rsf2.food_id)
                FROM recipe_step rs2
                LEFT JOIN recipe_step_food rsf2
                    ON rsf2.recipe_step_id = rs2.id
                WHERE rs2.recipe_id = f.id
            ) AS ingredient_count,
            CASE WHEN fl.locale_id <> ?3
                THEN NULL
                ELSE fl.locale_id
            END AS locale_id,
            (
                -- инфа об ингридиенте, который явл. рецептом
                SELECT
                    SUM(rs2.ingredients_mass)
                FROM recipe_step rs2
                WHERE rs2.recipe_id = f.id
            ) AS ingredients_mass
        FROM recipe_step rs
        INNER JOIN recipe_step_food rsf
            ON rsf.recipe_step_id = rs.id
        LEFT JOIN food f
            ON f.id = rsf.food_id
        LEFT JOIN food_localisation fl
            ON fl.food_id = f.id
        LEFT JOIN food_localisation_fts5 fl_fts5
            ON fl_fts5.rowid = fl.fl_fts5_rowid
        WHERE
            rs.recipe_id = ?4
            AND {0}
        ORDER BY f.id ASC
    ),
    partitioned_list AS (
        SELECT
            al.*,
            row_number() OVER (
                PARTITION BY al.id
                ORDER BY al.locale_id ASC NULLS LAST
            ) AS rn
        FROM filtered_list al
    ),
    the_list AS (
        SELECT
            id,
            food_id,
            food_title,
            canonical_mass_g,
            ingredient_count,
            ingredients_mass
        FROM partitioned_list
        WHERE rn = 1
        ORDER BY food_title ASC
    )
    SELECT COUNT(the_list.id), NULL, NULL, NULL, NULL, NULL FROM the_list
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT ?1 OFFSET ?2)
    )sql";

    // todo check int static_cast
    const auto per_page = static_cast<int>(pagination.per_page());
    const auto offset =
        static_cast<int>(pagination.per_page() * pagination.current_page());

    // fmt::println(sql, match_title, where);
    ok(stmt.prepare(fmt::format(sql, where)))                     //
        && ok(stmt.bind(1, per_page))                             //
        && ok(stmt.bind(2, offset))                               //
        && ok(stmt.bind(3, query.locale_id, sqlw::Type::SQL_INT)) //
        && ok(stmt.bind(4, query.food_id, sqlw::Type::SQL_INT))   //
        && ok(stmt.bind(5, match_title, sqlw::Type::SQL_TEXT));

    return {
        LengthContainer{pagination.per_page() * field_count}, stmt.status()};
}

static bool check_page(const wholth_Page* const page)
{
    return nullptr != page && PageType::INGREDIENT == page->data.index();
}

extern "C" wholth_Error wholth_pages_ingredient(
    wholth_Page** page,
    uint64_t per_page)
{
    auto err = wholth_pages_new(page);

    if (!wholth_error_ok(&err))
    {
        return err;
    }

    wholth::pages::Ingredient page_data{.query = {}, .container = {}};
    page_data.container.view.resize(per_page);

    **page = {per_page, page_data};

    return wholth_Error_OK;
}

extern "C" const wholth_IngredientArray wholth_pages_ingredient_array(
    const wholth_Page* const page)
{
    if (!check_page(page) || page->pagination.span_size() == 0)
    {
        return {nullptr, 0};
    }

    const auto& vector =
        std::get<PageType::INGREDIENT>(page->data).container.view;

    assertm(
        vector.size() >= page->pagination.span_size(),
        "You done goofed here wholth_pages_ingredient_array() [1]!");

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
        return wholth::utils::from_error_code(INGREDIENT_PAGE_TYPE_MISMATCH);
    }

    auto& query = std::get<PageType::INGREDIENT>(page->data).query;

    std::error_code ec{};
    switch (field)
    {
    case TITLE:
        query.title = wholth::utils::to_string_view(value);
        break;
    case LOCALE_ID: {
        const auto id = wholth::utils::to_string_view(value);
        if (!wholth::utils::is_valid_id(id))
        {
            ec = INGREDIENT_PAGE_BAD_LOCALE_ID;
        }
        else
        {
            query.locale_id = id;
        }
        break;
    }
    case FOOD_ID: {
        const auto id = wholth::utils::to_string_view(value);
        if (!wholth::utils::is_valid_id(id))
        {
            ec = INGREDIENT_PAGE_BAD_FOOD_ID;
        }
        else
        {
            query.food_id = id;
        }
        break;
    }
    }

    return ec ? wholth::utils::from_error_code(ec) : wholth_Error_OK;
}

extern "C" wholth_Error wholth_pages_ingredient_title(
    wholth_Page* const page,
    wholth_StringView title)
{
    return set_model_field(page, ModelField::TITLE, title);
}

extern "C" wholth_Error wholth_pages_ingredient_food_id(
    wholth_Page* const page,
    wholth_StringView food_id)
{
    return set_model_field(page, ModelField::FOOD_ID, food_id);
}

extern "C" wholth_Error wholth_pages_ingredient_locale_id(
    wholth_Page* const page,
    wholth_StringView locale_id)
{
    return set_model_field(page, ModelField::LOCALE_ID, locale_id);
}
