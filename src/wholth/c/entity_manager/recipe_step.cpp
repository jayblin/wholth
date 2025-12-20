#include "wholth/c/entity/recipe_step.h"
#include "db/db.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "utils/time_to_seconds.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/internal.hpp"
#include "wholth/entity_manager/recipe_step.hpp"
#include "wholth/entity_manager/food.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <cassert>

static_assert(nullptr == (void*)NULL);

using ::utils::time_to_seconds;
using wholth::c::internal::ec_to_error;
using wholth::entity_manager::recipe_step::Code;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;

static auto upsert_description(const wholth_RecipeStep* step) -> std::error_code
{
    return sqlw::Transaction{&db::connection()}(
        R"sql(
        INSERT INTO recipe_step_localisation (recipe_step_id, locale_id, description)
        SELECT
            ?1,
            ai.value,
            ?2
        FROM app_info ai
        WHERE ai.field = 'default_locale_id'
        ON CONFLICT(recipe_step_id, locale_id) DO UPDATE
            SET description = ?2
        )sql",
        std::array<sqlw::Statement::bindable_t, 2>{{
            {to_string_view(step->id), sqlw::Type::SQL_INT},
            {to_string_view(step->description), sqlw::Type::SQL_TEXT},
        }});
}

extern "C" auto wholth_em_recipe_step_insert(
    wholth_RecipeStep* const step,
    const wholth_Food* const recipe,
    wholth_Buffer* const buffer) -> wholth_Error
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == step)
    {
        return ec_to_error(
            wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL,
            buffer);
    }

    if (nullptr == recipe)
    {
        return ec_to_error(
            wholth::entity_manager::food::Code::FOOD_NULL, buffer);
    }

    const auto food_id = to_string_view(recipe->id);

    if (!is_valid_id(food_id))
    {
        return ec_to_error(
            wholth::entity_manager::food::Code::FOOD_INVALID_ID, buffer);
    }

    if (nullptr == step->time.data)
    {
        return ec_to_error(
            wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL_TIME,
            buffer);
    }

    std::string seconds;

    const auto seconds_err =
        time_to_seconds(to_string_view(step->time), seconds);

    if (::utils::time_to_seconds_Code::OK != seconds_err)
    {
        return ec_to_error(seconds_err, buffer);
    }

    if (nullptr == step->description.data)
    {
        return ec_to_error(Code::RECIPE_STEP_NULL_DESCRIPTION, buffer);
    }

    std::error_code ec;

    std::tuple<std::string, std::string> rs_id_and_rsl_id{"", ""};
    size_t idx = 0;
    ec = sqlw::Statement{&db::connection()}(
        "SELECT rs.id, rsl.locale_id "
        "FROM recipe_step rs "
        "LEFT JOIN recipe_step_localisation rsl "
        " ON rsl.recipe_step_id = rs.id "
        "    AND rsl.locale_id = (SELECT value FROM app_info WHERE field = "
        "'default_locale_id') "
        "WHERE recipe_id = ?1",
        [&rs_id_and_rsl_id, &idx](sqlw::Statement::ExecArgs a) {
            idx++;
            if (1 == idx)
            {
                std::get<0>(rs_id_and_rsl_id) = a.column_value;
            }
            else
            {
                std::get<1>(rs_id_and_rsl_id) = a.column_value;
            }
        },
        std::array<sqlw::Statement::bindable_t, 1>{{
            {food_id, sqlw::Type::SQL_INT},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    auto& step_id = std::get<0>(rs_id_and_rsl_id);
    const auto& locale_id = std::get<1>(rs_id_and_rsl_id);

    if (!step_id.empty() && !locale_id.empty())
    {
        return ec_to_error(
            wholth::entity_manager::recipe_step::Code::
                RECIPE_STEP_ALREADY_EXISTS,
            buffer);
    }

    if (step_id.empty())
    {
        ec = sqlw::Transaction{&db::connection()}(
            R"sql(
            INSERT INTO recipe_step (recipe_id, seconds)
            VALUES (?1, ?2)
            RETURNING id
            )sql",
            [&](sqlw::Statement::ExecArgs a) { step_id = a.column_value; },
            std::array<sqlw::Statement::bindable_t, 2>{{
                {food_id, sqlw::Type::SQL_INT},
                {seconds, sqlw::Type::SQL_INT},
            }});
    }

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(Code::RECIPE_STEP_INSERT_FAILURE, buffer);
    }

    wholth_buffer_move_data_to(buffer, &step_id);
    step->id = wholth_buffer_view(buffer);

    ec = upsert_description(step);

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(
            Code::RECIPE_STEP_LOCALISATION_UPDATE_FAILURE, buffer);
    }

    return wholth_Error_OK;
}

extern "C" wholth_Error wholth_em_recipe_step_update(
    const wholth_RecipeStep* const step,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == step)
    {
        return ec_to_error(
            wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL,
            buffer);
    }

    const auto provided_step_id = to_string_view(step->id);
    if (!is_valid_id(provided_step_id))
    {
        return ec_to_error(Code::RECIPE_STEP_INVALID_ID, buffer);
    }

    std::string seconds;
    if (step->time.data != nullptr)
    {
        const auto seconds_err =
            time_to_seconds(to_string_view(step->time), seconds);

        if (::utils::time_to_seconds_Code::OK != seconds_err)
        {
            return ec_to_error(seconds_err, buffer);
        }
    }

    std::error_code ec;

    if (step->time.data != nullptr)
    {
        ec = sqlw::Transaction{&db::connection()}(
            R"sql(
                UPDATE recipe_step
                SET seconds = ?1
                WHERE id = ?2
                )sql",
            std::array<sqlw::Statement::bindable_t, 2>{{
                {seconds, sqlw::Type::SQL_INT},
                {provided_step_id, sqlw::Type::SQL_INT},
            }});

        if (sqlw::status::Condition::OK != ec)
        {
            // push_and_get(ec, buffer);
            return ec_to_error(Code::RECIPE_STEP_UPDATE_FAILURE, buffer);
        }
    }

    if (step->description.data != nullptr)
    {
        ec = upsert_description(step);

        if (sqlw::status::Condition::OK != ec)
        {
            // push_and_get(ec, buffer);
            return ec_to_error(
                Code::RECIPE_STEP_LOCALISATION_UPDATE_FAILURE, buffer);
        }
    }

    return wholth_Error_OK;
}
