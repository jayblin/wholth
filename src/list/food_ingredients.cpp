#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/utils.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/status.hpp"
#include "wholth/utils.hpp"

template <>
std::error_code wholth::check_query(const wholth::model::FoodIngredients& query)
{
    using SC = wholth::status::Code;

    if (query.ctx.locale_id.size() == 0 ||
        !sqlw::utils::is_numeric(query.ctx.locale_id))
    {
        return SC::INVALID_LOCALE_ID;
    }

    if (query.food_id.size() == 0 || !sqlw::utils::is_numeric(query.food_id))
    {
        return SC::INVALID_FOOD_ID;
    }

    return SC::OK;
}

template <>
sqlw::Statement wholth::prepare_fill_span_statement(
    const wholth::model::FoodIngredients& query,
    size_t span_size,
    sqlw::Connection& db_con)
{
    using namespace wholth::utils;

    constexpr std::string_view sql = R"sql(
    WITH the_list as (
        SELECT
            f.id AS food_id,
            COALESCE(fl.title, '[N/A]') AS food_title,
            rsf.canonical_mass AS canonical_mass_g,
            (
                SELECT
                    COUNT(rsf2.food_id)
                FROM recipe_step rs2
                LEFT JOIN recipe_step_food rsf2
                    ON rsf2.recipe_step_id = rs2.id
                WHERE rs2.recipe_id = f.id
            ) AS ingredient_count
        FROM recipe_step rs
        INNER JOIN recipe_step_food rsf
            ON rsf.recipe_step_id = rs.id
        LEFT JOIN food f
            ON f.id = rsf.food_id
        LEFT JOIN food_localisation fl
            ON fl.food_id = f.id AND fl.locale_id = ?2
        WHERE
            rs.recipe_id = ?1
        ORDER BY rs.recipe_id ASC
    )
    SELECT COUNT(the_list.food_id), NULL, NULL, NULL FROM the_list
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT ?3 OFFSET ?4)
    )sql";

    sqlw::Statement stmt{&db_con};

    ok(stmt.prepare(sql))                                             //
        && ok(stmt.bind(1, query.food_id, sqlw::Type::SQL_INT))       //
        && ok(stmt.bind(2, query.ctx.locale_id, sqlw::Type::SQL_INT)) //
        && ok(stmt.bind(3, static_cast<int>(span_size)))              //
        &&
        ok(stmt.bind(
            4, static_cast<int>(span_size * query.pagination.current_page())));

    return stmt;
}
