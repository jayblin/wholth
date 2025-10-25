#include "wholth/model/recipe_step.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/utils.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/status.hpp"
#include "wholth/utils.hpp"

template <>
std::error_code wholth::check_query(const wholth::model::RecipeStep& query)
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
sqlw::Statement wholth::prepare_fill_object_statement(
    const wholth::model::RecipeStep& query,
    sqlw::Connection& db_con)
{
    using namespace wholth::utils;

    constexpr std::string_view sql = R"sql(
    SELECT
        rs.id,
        COALESCE(
            seconds_to_readable_time(rt.step_seconds),
            '[N/A]'
        ) AS time,
        rsl.description
    FROM recipe_step rs
    LEFT JOIN recipe_step_localisation rsl
        ON rsl.recipe_step_id = rs.id
        AND rsl.locale_id = ?1
    WHERE
        rs.recipe_id = ?2
    )sql";

    sqlw::Statement stmt{&db_con};

    ok(stmt.prepare(sql))                                             //
        && ok(stmt.bind(1, query.food_id, sqlw::Type::SQL_INT))       //
        && ok(stmt.bind(2, query.ctx.locale_id, sqlw::Type::SQL_INT)) //
        ;

    return stmt;
}
