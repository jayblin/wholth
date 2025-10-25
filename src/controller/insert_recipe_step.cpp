#include "wholth/controller/insert_recipe_step.hpp"
#include "sqlw/transaction.hpp"
#include "utils/time_to_seconds.hpp"
#include "wholth/utils/is_valid_id.hpp"

namespace wholth::controller
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "insert_recipe_step";
    }

    std::string message(int ev) const override final
    {
        using Code = insert_recipe_step_Code;

        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        case Code::INVALID_RECIPE_TIME:
            return "invalid recipe time";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory insert_recipe_step_error_category{};
} // namespace wholth::controller

std::error_code wholth::controller::make_error_code(
    wholth::controller::insert_recipe_step_Code e)
{
    return {
        static_cast<int>(e),
        wholth::controller::insert_recipe_step_error_category};
}

auto wholth::controller::insert(
    const wholth::entity::RecipeStep& step,
    std::string& result_buffer,
    sqlw::Connection& db_con,
    // const wholth::Context& ctx,
    wholth::entity::locale::id_t locale_id,
    const wholth::entity::Food& food) -> std::error_code
{
    // todo test
    result_buffer = "";

    /* if (food.id != "" && !is_valid_id(food.id)) */
    if (!is_valid_id(food.id))
    {
        return wholth::status::Code::INVALID_FOOD_ID;
    }

    if (!is_valid_id(locale_id))
    {
        return wholth::status::Code::INVALID_LOCALE_ID;
    }

    std::string seconds;
    const auto seconds_err =
        ::utils::time_to_seconds(step.time, seconds);

    if (::utils::time_to_seconds_Code::OK != seconds_err)
    {
        return insert_recipe_step_Code::INVALID_RECIPE_TIME;
    }

    using bindable_t = sqlw::Statement::bindable_t;

    auto ec = sqlw::Transaction{&db_con}(
        R"sql(
            INSERT INTO recipe_step (recipe_id,seconds) VALUES (?1, ?2);
            INSERT INTO recipe_step_localisation (recipe_step_id,locale_id,description) VALUES (last_insert_rowid(), ?1, ?2) RETURNING recipe_step_id
            )sql",
        [&result_buffer](auto e) { result_buffer = e.column_value; },
        std::array<bindable_t, 4>{
            bindable_t{food.id, sqlw::Type::SQL_INT},
            bindable_t{seconds, sqlw::Type::SQL_INT},
            bindable_t{locale_id, sqlw::Type::SQL_INT},
            bindable_t{step.description, sqlw::Type::SQL_TEXT},
        });

    return ec;
}
