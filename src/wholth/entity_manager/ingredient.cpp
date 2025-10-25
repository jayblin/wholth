#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "sqlw/utils.hpp"
#include "wholth/entity_manager/ingredient.hpp"
#include "wholth/entity_manager/recipe_step.hpp"
#include "wholth/utils/is_valid_id.hpp"

namespace wholth::entity_manager::ingredient
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "entity_manager::ingredient";
    }

    std::string message(int ev) const override final
    {
        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        case Code::INGREDIENT_INVALID_ID:
            return "INGREDIENT_INVALID_ID";
        case Code::INGREDIENT_INVALID_MASS:
            return "INGREDIENT_INVALID_MASS";
        case Code::INGREDIENT_POSTCONDITION_FAILED:
            // Either user provided recipe_step_id which does not exist or
            // recipe_id is equal to ingredient_id.
            return "INGREDIENT_POSTCONDITION_FAILED";
        case Code::INGREDIENT_IS_NULL:
            return "INGREDIENT_IS_NULL";
        case Code::INGREDIENT_INVALID_FOOD_ID:
            return "INGREDIENT_INVALID_FOOD_ID";
        }

        return "(unrecognized error)";
    }
};

// todo: maybe use global (wholth app) error category?
const ErrorCategory error_category{};
} // namespace wholth::entity_manager::ingredient

std::error_code wholth::entity_manager::ingredient::make_error_code(
    wholth::entity_manager::ingredient::Code e)
{
    return {
        static_cast<int>(e),
        wholth::entity_manager::ingredient::error_category};
}

// auto wholth::entity_manager::ingredient::insert(
//     const wholth::entity::Ingredient& ingredient,
//     std::string& result_buffer,
//     sqlw::Connection& db_con,
//     const wholth::entity::RecipeStep& step) -> std::error_code
// {
//     using namespace wholth::utils;
//
//     result_buffer = "";
//
//     if (!is_valid_id(ingredient.food_id))
//     {
//         return Code::INGREDIENT_INVALID_ID;
//     }
//
//     if (ingredient.canonical_mass_g.size() == 0 ||
//         !sqlw::utils::is_numeric(ingredient.canonical_mass_g))
//     {
//         return Code::INGREDIENT_INVALID_MASS;
//     }
//
//     if (!is_valid_id(step.id))
//     {
//         return recipe_step::Code::RECIPE_STEP_INVALID_ID;
//     }
//
//     using bindable_t = sqlw::Statement::bindable_t;
//
//     const auto ec = sqlw::Transaction{&db_con}(
//         R"sql(
//         INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) 
//         SELECT
//             rs.id,
//             ?2 AS food_id,
//             ?3 AS canonical_mass
//         FROM recipe_step rs
//         WHERE rs.id = ?1 AND rs.recipe_id <> ?2
//         LIMIT 1
//         RETURNING rs.id
//         )sql",
//         [&result_buffer](auto e) { result_buffer = e.column_value; },
//         std::array<bindable_t, 3>{{
//             {step.id, sqlw::Type::SQL_INT},
//             {ingredient.food_id, sqlw::Type::SQL_INT},
//             {ingredient.canonical_mass_g, sqlw::Type::SQL_DOUBLE},
//         }});
//
//     if (sqlw::status::Condition::OK != ec)
//     {
//         return ec;
//     }
//
//     if (result_buffer.size() == 0)
//     {
//         return Code::INGREDIENT_POSTCONDITION_FAILED;
//     }
//
//     ingredient.id
//
//     return Code::OK;
// }
