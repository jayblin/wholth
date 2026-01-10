#include "fmt/core.h"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/c/entity/food.h"
#include "wholth/c/pages/food.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/error.hpp"
#include "wholth/pages/code.hpp"
#include "wholth/pages/food.hpp"
#include "wholth/pagination.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/length_container.hpp"
#include "wholth/utils/str_replace.hpp"
#include "wholth/utils/to_error.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <sstream>
#include <cassert>
#include <system_error>

// todo
// - move this file to wholth/c/pages/food.cpp

static_assert(wholth::entity::is_food<wholth_Food>);
static_assert(wholth::entity::count_fields<wholth_Food>() == 4);

using SC = wholth::pages::Code;
using LengthContainer = wholth::entity::LengthContainer;
using wholth::utils::is_valid_id;

constexpr auto field_count = wholth::entity::count_fields<wholth_Food>();

template <>
struct wholth::error::is_error_code_enum<wholth_pages_food_Code>
    : std::true_type
{
};

auto wholth::pages::hydrate(
    Food& food,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void
{
    auto& buffer = food.container.buffer;
    auto& vec = food.container.view;

    assert(vec.size() > index);

    wholth::utils::extract(vec[index].id, lc, buffer);
    wholth::utils::extract(vec[index].title, lc, buffer);
    wholth::utils::extract(vec[index].preparation_time, lc, buffer);
    wholth::utils::extract(vec[index].top_nutrient, lc, buffer);
}

constexpr std::string_view sql_top_nutrient =
    R"sql(top_nutrients(value, recipe_id, position) AS NOT MATERIALIZED (
    SELECT
        fn.value || ' ' || COALESCE(n.unit, ''),
        fn.food_id,
        n.position
    FROM food_nutrient fn
    LEFT JOIN nutrient n
        ON n.id = fn.nutrient_id
    ORDER BY n.position ASC
))sql";

constexpr std::string_view sql_generic =
    R"sql(
accumulated_list AS (
    SELECT
        f.id AS id,
        fl_fts5.title AS title,
        seconds_to_readable_time(rs.seconds) AS prepartion_time,
        COALESCE(tp.value, '[N/A]') AS top_nutrient,
        CASE WHEN fl.locale_id <> ?3
            THEN NULL
            ELSE fl.locale_id
        END AS locale_id,
        f.created_at AS created_at
        -- fl_fts5.description AS description
    FROM food f
    LEFT JOIN recipe_step rs
        ON rs.recipe_id = f.id
    INNER JOIN food_localisation fl
        ON fl.food_id = f.id
    INNER JOIN food_localisation_fts5 fl_fts5
        ON fl_fts5.rowid = fl.fl_fts5_rowid
    LEFT JOIN top_nutrients tp
        ON tp.recipe_id = f.id
            AND tp.position = 10
    WHERE {0}
),
partitioned_list AS (
    SELECT
        al.*,
        row_number() OVER (
            PARTITION BY al.id
            ORDER BY al.locale_id ASC NULLS LAST
        ) AS rn
    FROM accumulated_list al
),
the_list AS (
    SELECT id, title, prepartion_time, top_nutrient
    FROM partitioned_list
    WHERE rn = 1
    ORDER BY created_at DESC, title ASC
))sql";

constexpr std::string_view sql_result_union = R"sql(
SELECT COUNT(the_list.id), NULL, NULL, NULL FROM the_list
UNION ALL
SELECT * FROM (SELECT * FROM the_list LIMIT ?1 OFFSET ?2))sql";

static auto prepare_food_stmt_by_id(
    sqlw::Statement& stmt,
    const wholth::pages::FoodQuery& query) -> std::error_code
{
    using wholth::utils::ok;

    if (!is_valid_id(query.id))
    {
        return wholth_pages_food_Code::FOOD_PAGE_BAD_FOOD_ID;
    }

    const std::string sql_filtered_list =
        fmt::format(sql_generic, " f.id = ?4 ");
    const std::string sql = fmt::format(
        "WITH {0},{1} {2}",
        sql_top_nutrient,
        sql_filtered_list,
        sql_result_union);

    ok(stmt.prepare(sql)) //
        && ok(stmt.bind(4, query.id, sqlw::Type::SQL_INT));

    return stmt.status();
}

static auto prepare_food_stmt_generic(
    sqlw::Statement& stmt,
    const wholth::pages::FoodQuery& query) -> std::error_code
{
    using wholth::utils::ok;

    std::string where = " 1=1 ";
    std::string match_title = "";

    if (!query.title.empty())
    {
        if (query.title.size() < 3)
        {
            return wholth_pages_food_Code::FOOD_PAGE_TITLE_TOO_SHORT;
        }

        where = " food_localisation_fts5 MATCH ?4 ";
        match_title = fmt::format(
            "{{title}}:({0})",
            wholth::utils::str_replace(query.title, ",", " OR "));
    }

    const std::string sql_filtered_list = fmt::format(sql_generic, where);
    const std::string sql = fmt::format(
        "WITH {0},{1} {2}",
        sql_top_nutrient,
        sql_filtered_list,
        sql_result_union);

    ok(stmt.prepare(sql));

    if (!match_title.empty())
    {
        ok(stmt.bind(4, match_title, sqlw::Type::SQL_TEXT));
    }

    return stmt.status();
}

constexpr std::string_view sql_by_ingredients =
    R"sql(food_tree AS (
    SELECT
        1 AS lvl,
        fl.food_id AS food_id,
        0 AS seconds
    FROM food_localisation_fts5 fl_fts5
    INNER JOIN food_localisation fl
        ON fl.fl_fts5_rowid = fl_fts5.rowid
    WHERE
        food_localisation_fts5 MATCH ?4
    UNION
    SELECT
        ft.lvl + 1 AS lvl,
        rs.recipe_id AS food_id,
        rs.seconds AS seconds
    FROM food_tree ft
    INNER JOIN recipe_step_food rsf
        ON rsf.food_id = ft.food_id
    INNER JOIN recipe_step rs
        ON rs.id = rsf.recipe_step_id
    WHERE lvl < 6
),
accumulated_list AS (
    SELECT
        fl.food_id AS id,
        fl_fts5.title AS title,
        COALESCE(
            seconds_to_readable_time(ft.seconds),
            '[N/A]'
        ) AS prepartion_time,
        COALESCE(tp.value, '[N/A]') AS top_nutrient,
        CASE WHEN fl.locale_id <> ?3
            THEN NULL
            ELSE fl.locale_id
        END AS locale_id,
        ft.lvl
    FROM food_tree ft
    INNER JOIN food_localisation fl
        ON fl.food_id = ft.food_id
    INNER JOIN food_localisation_fts5 fl_fts5
        ON fl.fl_fts5_rowid = fl_fts5.rowid
    LEFT JOIN top_nutrients tp
        ON tp.recipe_id = ft.food_id
            AND tp.position = 10
),
partitioned_list AS (
    SELECT
        al.*,
        row_number() OVER (
            PARTITION BY id
            ORDER BY locale_id ASC NULLS LAST
        ) AS rn
    FROM accumulated_list al
),
the_list AS (
    SELECT id, title, prepartion_time, top_nutrient
    FROM partitioned_list
    WHERE rn = 1 AND lvl <> 1
    ORDER BY id DESC
))sql";

static auto prepare_food_stmt_by_ingredients(
    sqlw::Statement& stmt,
    const wholth::pages::FoodQuery& query) -> std::error_code
{
    using wholth::utils::ok;

    if (query.ingredients.size() < 3)
    {
        return wholth_pages_food_Code::FOOD_PAGE_INGREDIENT_LIST_TOO_SHORT;
    }

    const std::string sql = fmt::format(
        "WITH RECURSIVE {0}, {1} {2}",
        sql_top_nutrient,
        sql_by_ingredients,
        sql_result_union);

    const auto ingredients = fmt::format(
        "{{title}}:({0})",
        wholth::utils::str_replace(query.ingredients, ",", " OR "));
    // std::cout << "ingredients: " << ingredients << '\n';

    ok(stmt.prepare(sql)) //
        && ok(stmt.bind(4, ingredients, sqlw::Type::SQL_TEXT));

    return stmt.status();
}

auto wholth::pages::prepare_food_stmt(
    sqlw::Statement& stmt,
    const wholth::pages::FoodQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    using wholth::utils::ok;

    if (!is_valid_id(query.locale_id))
    {
        return {
            LengthContainer{}, wholth_pages_food_Code::FOOD_PAGE_BAD_LOCALE_ID};
    }

    std::error_code ec{};

    if (!query.id.empty())
    {
        ec = prepare_food_stmt_by_id(stmt, query);
    }
    else if (!query.ingredients.empty())
    {
        ec = prepare_food_stmt_by_ingredients(stmt, query);
    }
    else
    {
        ec = prepare_food_stmt_generic(stmt, query);
    }

    if (ec)
    {
        return {LengthContainer{}, ec};
    }

    // todo check bounds
    const auto per_page = static_cast<int>(pagination.per_page());
    const auto offset =
        static_cast<int>(pagination.per_page() * pagination.current_page());

    ok(stmt.bind(1, per_page))      //
        && ok(stmt.bind(2, offset)) //
        && ok(stmt.bind(3, query.locale_id, sqlw::Type::SQL_INT));

    // size_t j = 0;
    // stmt([&j](auto e) {
    //     if (0 == j % e.column_count)
    //     {
    //         std::cout << "------\n";
    //     }
    //     j++;
    //     std::cout << e.column_name << ": " << e.column_value << '\n';
    // });

    return {LengthContainer{per_page * field_count}, stmt.status()};
}
