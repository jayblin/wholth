#include "wholth/c/entity/food.h"
#include "db/db.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity_manager/food.h"
#include "wholth/c/forward.h"
#include "wholth/c/internal.hpp"
#include "wholth/entity_manager/food.hpp"
#include "wholth/status.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/prepend_sql_params.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <sstream>

using wholth::c::internal::ec_to_error;
using wholth::entity_manager::food::Code;
using wholth::utils::current_time_and_date;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;

static auto& g_context = wholth::c::internal::global_context();

// todo: move erro_code stuff to wholth/entity/manager/food.cpp
namespace wholth::entity_manager::food
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "entity_manager::food";
    }

    std::string message(int ev) const override final
    {
        using Code = wholth::entity_manager::food::Code;

        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        case food::Code::FOOD_INVALID_ID:
            return "INVALID_FOOD_ID";
        case food::Code::FOOD_NULL:
            return "NULL_FOOD";
        // case food::Code::NULL_DETAILS:
        //     return "NULL_DETAILS";
        case Code::FOOD_NULL_TITLE:
            return "FOOD_NULL_TITLE";
        case Code::FOOD_NULL_DESCRIPTION:
            return "FOOD_NULL_DESCRIPTION";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory error_category{};
} // namespace wholth::entity_manager::food

std::error_code wholth::entity_manager::food::make_error_code(
    wholth::entity_manager::food::Code e)
{
    return {static_cast<int>(e), wholth::entity_manager::food::error_category};
}

extern "C" auto wholth_em_food_insert(
    wholth_Food* const food,
    wholth_Buffer* const buffer) -> wholth_Error
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == food)
    {
        return ec_to_error(Code::FOOD_NULL, buffer);
    }

    if (nullptr == food->title.data || 0 == food->title.size)
    {
        return ec_to_error(Code::FOOD_NULL_TITLE, buffer);
    }

    // if (nullptr == food->description.data || 0 == food->description.size)
    // {
    //     return push_and_get(Code::FOOD_NULL_DESCRIPTION, buffer);
    // }

    const std::string now = current_time_and_date();

    std::string result_id;
    const std::array<sqlw::Statement::bindable_t, 3> params{
        {{now, sqlw::Type::SQL_TEXT},
         {to_string_view(food->title), sqlw::Type::SQL_TEXT},
         {to_string_view(food->description),
          (nullptr == food->description.data || 0 == food->description.size)
              ? sqlw::Type::SQL_NULL
              : sqlw::Type::SQL_TEXT}}};

    const auto ec = sqlw::Transaction{&db::connection()}(
        R"sql(
        INSERT INTO food (created_at) VALUES (?1);
        INSERT INTO food_localisation (food_id, locale_id, title, description) 
        -- VALUES (last_insert_rowid(), ?1, lower(trim(?2)), ?3)
        SELECT 
            last_insert_rowid(), ai.value, lower(trim(?1)), ?2
        FROM app_info AS ai
        WHERE ai.field = 'default_locale_id'
        RETURNING food_id
        )sql",
        [&](auto e) { result_id = e.column_value; },
        params);

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    wholth_buffer_move_data_to(buffer, &result_id);

    food->id = wholth_buffer_view(buffer);
    // wholth_StringView{.data = result_id.data(), .size = result_id.size()};

    return wholth_Error_OK;
}

extern "C" auto wholth_em_food_update(
    const wholth_Food* const food,
   wholth_Buffer* const buffer/*,
    const wholth_FoodDetails* deets*/) -> wholth_Error
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == food)
    {
        return ec_to_error(Code::FOOD_NULL, buffer);
    }

    // if (nullptr == deets)
    // {
    //     return
    //     push_and_get(wholth::entity_manager::food::Code::NULL_DETAILS);
    // }

    const auto id = to_string_view(food->id);

    if (!is_valid_id(id))
    {
        return wholth::c::internal::ec_to_error(Code::FOOD_INVALID_ID, buffer);
        // return wholth::c::internal::push_and_get(
        //     wholth::entity_manager::food::Code::INVALID_FOOD_ID);
    }

    std::error_code ec;

    std::vector<sqlw::Statement::bindable_t> params;
    params.reserve(3);
    params.emplace_back(id, sqlw::Type::SQL_INT);

    std::stringstream ss;
    ss << "UPDATE food_localisation SET ";

    wholth::utils::prepend_sql_params(
        std::array<wholth::utils::extended_param_t, 2>{{
            {"title", food->title, sqlw::Type::SQL_TEXT, "lower(trim(?{}))"},
            // {"description", deets->description, sqlw::Type::SQL_TEXT, {}},
            {"description", food->description, sqlw::Type::SQL_TEXT, {}},
        }},
        params,
        ss);

    ss << " WHERE food_id = ?1 AND locale_id = (SELECT value FROM app_info "
          "WHERE field = 'default_locale_id')";

    sqlw::Transaction transaction{&db::connection()};
    ec = transaction(ss.str(), params);

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}

extern "C" auto wholth_em_food_delete(
    const wholth_Food* const food,
    wholth_Buffer* const buffer) -> wholth_Error
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == food)
    {
        return ec_to_error(Code::FOOD_NULL, buffer);
    }

    const auto id = to_string_view(food->id);

    if (!is_valid_id(id))
    {
        return wholth::c::internal::ec_to_error(Code::FOOD_INVALID_ID, buffer);
    }

    const auto ec = sqlw::Transaction{&db::connection()}(
        "DELETE FROM food WHERE id = ?1",
        std::array<sqlw::Statement::bindable_t, 1>{
            {{id, sqlw::Type::SQL_INT}}});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}
