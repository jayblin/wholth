#include "wholth/c/entity_manager/ingredient.h"
#include "db/db.hpp"
#include "sqlw/transaction.hpp"
#include "sqlw/utils.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/forward.h"
#include "wholth/c/internal.hpp"
#include "wholth/entity_manager/ingredient.hpp"
#include "wholth/entity_manager/recipe_step.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <cassert>

using wholth::c::internal::ec_to_error;
using wholth::entity_manager::ingredient::Code;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;

static_assert(nullptr == (void*)NULL);

extern "C" auto wholth_em_ingredient_insert(
    wholth_Ingredient* const ing,
    const wholth_RecipeStep* const step,
    wholth_Buffer* const buffer) -> wholth_Error
{
    if (nullptr == step)
    {
        return ec_to_error(
            wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL,
            buffer);
    }

    if (nullptr == ing)
    {
        return ec_to_error(Code::INGREDIENT_IS_NULL, buffer);
    }

    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    const auto food_id = to_string_view(ing->food_id);

    if (!is_valid_id(food_id))
    {
        return ec_to_error(Code::INGREDIENT_INVALID_FOOD_ID, buffer);
    }

    const auto mass = to_string_view(ing->canonical_mass_g);

    if (mass.size() == 0 || !sqlw::utils::is_numeric(mass))
    {
        return ec_to_error(Code::INGREDIENT_INVALID_MASS, buffer);
    }

    const auto step_id = to_string_view(step->id);

    if (!is_valid_id(step_id))
    {
        return ec_to_error(
            wholth::entity_manager::recipe_step::Code::RECIPE_STEP_INVALID_ID,
            buffer);
    }

    std::string result_id;

    using bindable_t = sqlw::Statement::bindable_t;

    const auto ec = sqlw::Transaction{&db::connection()}(
        R"sql(
        INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) 
        SELECT
            rs.id,
            ?2 AS food_id,
            ?3 AS canonical_mass
        FROM recipe_step rs
        WHERE rs.id = ?1 AND rs.recipe_id <> ?2
        LIMIT 1
        RETURNING id
        )sql",
        [&result_id](auto e) { result_id = e.column_value; },
        std::array<bindable_t, 3>{{
            {step_id, sqlw::Type::SQL_INT},
            {food_id, sqlw::Type::SQL_INT},
            {mass, sqlw::Type::SQL_DOUBLE},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    if (result_id.size() == 0 || !is_valid_id(result_id))
    {
        return ec_to_error(Code::INGREDIENT_POSTCONDITION_FAILED, buffer);
    }

    wholth_buffer_move_data_to(buffer, &result_id);

    ing->id = wholth_buffer_view(buffer);

    return wholth_Error_OK;
}

extern "C" auto wholth_em_ingredient_update(
    const wholth_Ingredient* const ing,
    const wholth_RecipeStep* const step,
    wholth_Buffer* const buffer) -> wholth_Error
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == ing)
    {
        return ec_to_error(Code::INGREDIENT_IS_NULL, buffer);
    }

    if (nullptr == step)
    {
        return ec_to_error(
            wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL,
            buffer);
    }

    const auto mass = to_string_view(ing->canonical_mass_g);

    if (mass.size() == 0 || !sqlw::utils::is_numeric(mass))
    {
        return ec_to_error(Code::INGREDIENT_INVALID_MASS, buffer);
    }

    const auto id = to_string_view(ing->id);

    if (!is_valid_id(id))
    {
        return ec_to_error(
            wholth::entity_manager::ingredient::Code::INGREDIENT_INVALID_ID,
            buffer);
    }

    const auto ec = sqlw::Transaction{&db::connection()}(
        R"sql(
        UPDATE recipe_step_food
        SET canonical_mass = ?2
        WHERE
            id = ?1
        )sql",
        std::array<sqlw::Statement::bindable_t, 2>{{
            {id, sqlw::Type::SQL_INT},
            {mass, sqlw::Type::SQL_DOUBLE},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}
