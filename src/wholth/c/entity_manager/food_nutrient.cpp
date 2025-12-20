
#include "db/db.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "sqlw/utils.hpp"
#include "wholth/c/forward.h"
#include "wholth/c/internal.hpp"
#include "wholth/entity_manager/food.hpp"
#include "wholth/entity_manager/nutrient.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_string_view.hpp"
#include "wholth/c/entity_manager/food_nutrient.h"
#include <cassert>

using wholth::c::internal::ec_to_error;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;

static_assert(nullptr == (void*)NULL);

extern "C" wholth_Error wholth_em_food_nutrient_upsert(
    const wholth_Food* food,
    const wholth_Nutrient* nutrient,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == food)
    {
        return ec_to_error(
            wholth::entity_manager::food::Code::FOOD_NULL, buffer);
    }

    if (nullptr == nutrient)
    {
        return ec_to_error(
            wholth::entity_manager::nutrient::Code::NUTRIENT_NULL, buffer);
    }

    const auto food_id = to_string_view(food->id);

    if (!is_valid_id(food_id))
    {
        return ec_to_error(
            wholth::entity_manager::food::Code::FOOD_INVALID_ID, buffer);
    }

    const auto n_id = to_string_view(nutrient->id);

    if (!is_valid_id(n_id))
    {
        return ec_to_error(
            wholth::entity_manager::nutrient::Code::NUTRIENT_INVALID_ID,
            buffer);
    }

    const auto value = to_string_view(nutrient->value);

    if (value.empty() || !sqlw::utils::is_numeric(value))
    {
        return ec_to_error(
            wholth::entity_manager::nutrient::Code::NUTRIENT_INVALID_VALUE,
            buffer);
    }

    const auto ec = sqlw::Transaction{&db::connection()}(
        "INSERT INTO food_nutrient (food_id, nutrient_id, value) "
        "VALUES (?1, ?2, ?3) "
        "ON CONFLICT(food_id, nutrient_id) DO UPDATE SET value=?3",
        std::array<sqlw::statement::internal::bindable_t, 3>{{
            {food_id, sqlw::Type::SQL_INT},
            {n_id, sqlw::Type::SQL_INT},
            {value, sqlw::Type::SQL_DOUBLE},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}

extern "C" wholth_Error wholth_em_food_nutrient_update_important(
    const wholth_Food* const food,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == food)
    {
        return ec_to_error(
            wholth::entity_manager::food::Code::FOOD_NULL, buffer);
    }

    if (nullptr == food->id.data || food->id.size <= 0)
    {
        return ec_to_error(
            wholth::entity_manager::food::Code::FOOD_INVALID_ID, buffer);
    }

    const auto food_id = to_string_view(food->id);

    if (!wholth::utils::is_valid_id(food_id))
    {
        return ec_to_error(
            wholth::entity_manager::food::Code::FOOD_INVALID_ID, buffer);
    }

    const auto ec = sqlw::Transaction{&db::connection()}(
        R"sql(
        WITH RECURSIVE
            cte AS (
                SELECT
                    n.id AS nutrient_id,
                    fn.value AS nutrient_value,
                    fn.food_id AS food_id,
                    n.position AS nutrient_pos,
                    rs.recipe_id AS recipe_id,
                    rsf.canonical_mass AS canonical_mass,
                    1 AS lvl
                FROM recipe_step_food rsf
                INNER JOIN food_nutrient fn
                    ON rsf.food_id = fn.food_id
                INNER JOIN recipe_step rs
                    ON rs.id = rsf.recipe_step_id
                INNER JOIN nutrient n
                    ON n.id = fn.nutrient_id
                WHERE
                    rs.recipe_id = ?1
                    AND n.position BETWEEN 10 AND 60
                UNION ALL
                SELECT
                    n.id AS nutrient_id,
                    fn.value AS nutrient_value,
                    rsf.food_id AS food_id,
                    n.position AS nutrient_pos,
                    rs.recipe_id AS recipe_id,
                    rsf.canonical_mass AS canonical_mass,
                    cte.lvl + 1 AS lvl
                FROM cte
                INNER JOIN recipe_step rs
                    ON rs.recipe_id = cte.food_id
                INNER JOIN recipe_step_food rsf
                    ON rsf.recipe_step_id = rs.id
                INNER JOIN food_nutrient fn
                    ON fn.food_id = rsf.food_id
                INNER JOIN nutrient n
                    ON n.id = fn.nutrient_id
                WHERE
                    n.position BETWEEN 10 AND 60
            ),
        grouped_nutrients as (
            SELECT
                nutrient_id,
                SUM(nutrient_value * canonical_mass) / SUM(canonical_mass) value_sum,
                COUNT(nutrient_id) value_count,
                COUNT(DISTINCT food_id) food_cnt,
                GROUP_CONCAT(food_id) food_ids,
                recipe_id
            from cte
            WHERE
                cte.lvl = 1
            GROUP by nutrient_id
        ),
        max_food_count as (
            SELECT
                MAX(food_cnt) val
            from grouped_nutrients
        )
        INSERT OR REPLACE INTO food_nutrient (food_id, nutrient_id, value)
        SELECT
            recipe_id,
            nutrient_id,
            value_sum
        FROM grouped_nutrients, max_food_count
        WHERE max_food_count.val = grouped_nutrients.food_cnt
        -- RETURNING (
        --     SELECT
        --         max_food_count.val || ',' || grouped_nutrients.food_cnt
        --     FROM grouped_nutrients, max_food_count
        --     WHERE
        --         max_food_count.val <> grouped_nutrients.food_cnt
        --     GROUP BY max_food_count.val
        -- ) AS missing_nutrients
    )sql",
        /* [&i](auto e) { */
        /*     std::cout << e.column_name << ": " << e.column_value << '\n'; */
        /*     i++; */
        /*     if (i % 7 == 0) */
        /*     { */
        /*         std::cout << "------------------\n"; */
        /*     } */
        /* }, */
        std::array<sqlw::Statement::bindable_t, 1>{{
            {food_id, sqlw::Type::SQL_INT},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}
