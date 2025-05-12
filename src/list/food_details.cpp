#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/utils.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/status.hpp"
#include "wholth/utils.hpp"

template <>
std::error_code wholth::check_query(const wholth::model::FoodDetails& query)
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
    const wholth::model::FoodDetails& query,
    sqlw::Connection& db_con)
{
    using namespace wholth::utils;

    constexpr std::string_view sql = R"sql(
    WITH the_deets as (
        SELECT
            COALESCE(fl.description, '[N/A]') AS description
        FROM food f
        LEFT JOIN food_localisation fl
            ON fl.food_id = f.id AND fl.locale_id = ?2
        WHERE
            f.id = ?1
        ORDER BY n.position ASC
    )
    SELECT * FROM (SELECT * FROM the_deets)
    )sql";

    sqlw::Statement stmt{&db_con};

    ok(stmt.prepare(sql))                                             //
        && ok(stmt.bind(1, query.food_id, sqlw::Type::SQL_INT))       //
        && ok(stmt.bind(2, query.ctx.locale_id, sqlw::Type::SQL_INT)) //
        ;

    return stmt;
}
