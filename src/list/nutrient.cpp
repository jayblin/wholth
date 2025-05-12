#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/utils.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/status.hpp"
#include "wholth/utils.hpp"

/* using FDQuery = wholth::list::nutrient::FoodDependentQuery; */
using FDQuery = wholth::model::NutrientsPage;

template <>
std::error_code wholth::check_query<FDQuery>(const FDQuery& query)
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
sqlw::Statement wholth::prepare_fill_span_statement<FDQuery>(
    const FDQuery& query,
    size_t span_size,
    sqlw::Connection& db_con)
{
    using namespace wholth::utils;

    constexpr std::string_view sql_tpl = R"sql(
    WITH the_list as (
        SELECT
            n.id,
            COALESCE(nl.title, '[N/A]') AS title,
            ROUND(fn.value, 1) AS value,
            n.unit,
            n.position
        FROM nutrient n
        INNER JOIN food_nutrient fn
            ON fn.nutrient_id = n.id AND fn.food_id = ?1
        LEFT JOIN nutrient_localisation nl
            ON nl.nutrient_id = n.id AND nl.locale_id = ?2
        WHERE
            1=1
            {0}
        ORDER BY n.position ASC
    )
    SELECT COUNT(the_list.id), NULL, NULL, NULL, NULL FROM the_list
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT ?3 OFFSET ?4)
    )sql";

    sqlw::Statement stmt{&db_con};

    const std::string sql = fmt::format(
        sql_tpl,
        query.title.size() > 0 ? "AND nl.title LIKE ?5" : ""
    );

    ok(stmt.prepare(sql))                                             //
        && ok(stmt.bind(1, query.food_id, sqlw::Type::SQL_INT))       //
        && ok(stmt.bind(2, query.ctx.locale_id, sqlw::Type::SQL_INT)) //
        && ok(stmt.bind(3, static_cast<int>(span_size)))              //
        &&
        ok(stmt.bind(
            4, static_cast<int>(span_size * query.pagination.current_page())));

    if (query.title.size() > 0) {
        std::string title_param_value = fmt::format("%{0}%", query.title);
        stmt.bind(5, title_param_value, sqlw::Type::SQL_TEXT);
    }

    return stmt;
}
