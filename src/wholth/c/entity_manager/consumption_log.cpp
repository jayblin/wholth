#include "db/db.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "sqlw/utils.hpp"
#include "utils/datetime.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity/consumption_log.h"
#include "wholth/c/entity_manager/consumption_log.h"
#include "wholth/c/forward.h"
#include "wholth/c/internal.hpp"
#include "wholth/entity_manager/consumption_log.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/prepend_sql_params.hpp"
#include "wholth/utils/to_string_view.hpp"

using fmt::detail::to_string_view;
using wholth::c::internal::push_and_get;
using wholth::entity_manager::consumption_log::Code;
using wholth::utils::current_time_and_date;
using wholth::utils::is_valid_id;

// todo: move erro_code stuff to wholth/entity/manager/consumption_log.cpp
namespace wholth::entity_manager::consumption_log
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "entity_manager::consumption_log";
    }

    std::string message(int ev) const override final
    {
        using Code = wholth::entity_manager::consumption_log::Code;

        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        case Code::CONSUMPTION_LOG_NULL:
            return "CONSUMPTION_LOG_NULL";
        case Code::CONSUMPTION_LOG_INVALID_ID:
            return "CONSUMPTION_LOG_INVALID_ID";
        case Code::CONSUMPTION_LOG_INVALID_FOOD_ID:
            return "CONSUMPTION_LOG_INVALID_FOOD_ID";
        case Code::CONSUMPTION_LOG_INVALID_MASS:
            return "CONSUMPTION_LOG_INVALID_MASS";
        case Code::CONSUMPTION_LOG_INVALID_DATE:
            return "CONSUMPTION_LOG_INVALID_DATE";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory error_category{};
} // namespace wholth::entity_manager::consumption_log

std::error_code wholth::entity_manager::consumption_log::make_error_code(
    wholth::entity_manager::consumption_log::Code e)
{
    return {
        static_cast<int>(e),
        wholth::entity_manager::consumption_log::error_category};
}

extern "C" wholth_Error wholth_em_consumption_log_insert(
    wholth_ConsumptionLog* const log,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == log)
    {
        return push_and_get(Code::CONSUMPTION_LOG_NULL, buffer);
    }

    const auto food_id = wholth::utils::to_string_view(log->food_id);
    if (food_id.empty() || !is_valid_id(food_id))
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_FOOD_ID, buffer);
    }

    if (nullptr == log->mass.data || 0 == log->mass.size)
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_MASS, buffer);
    }

    const auto mass = wholth::utils::to_string_view(log->mass);

    if (!sqlw::utils::is_numeric(mass))
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_MASS, buffer);
    }

    if (nullptr == log->consumed_at.data || 0 == log->consumed_at.size)
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_DATE, buffer);
    }

    const auto datetime = wholth::utils::to_string_view(log->consumed_at);

    if (!::utils::datetime::is_valid_sqlite_datetime(datetime))
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_DATE, buffer);
    }

    std::string id;
    const std::error_code ec = sqlw::Transaction{&db::connection()}(
        "INSERT INTO consumption_log (food_id, mass, consumed_at) "
        "VALUES (?1, ?2, ?3) RETURNING id",
        [&id](sqlw::Statement::ExecArgs e) { id = e.column_value; },
        std::array<sqlw::Statement::bindable_t, 3>{{
            {food_id, sqlw::Type::SQL_INT},
            {mass, sqlw::Type::SQL_DOUBLE},
            {datetime, sqlw::Type::SQL_TEXT},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return push_and_get(ec, buffer);
    }

    if (id.empty())
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_ID, buffer);
    }

    wholth_buffer_move_data_to(buffer, &id);
    log->id = wholth_buffer_view(buffer);

    return wholth_Error_OK;
}

extern "C" wholth_Error wholth_em_consumption_log_update(
    const wholth_ConsumptionLog* const log,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == log)
    {
        return push_and_get(Code::CONSUMPTION_LOG_NULL, buffer);
    }

    const auto id = wholth::utils::to_string_view(log->id);
    if (id.empty() || !is_valid_id(id))
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_ID, buffer);
    }

    const auto food_id = wholth::utils::to_string_view(log->food_id);
    if (food_id.empty() || !is_valid_id(food_id))
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_FOOD_ID, buffer);
    }

    if ((nullptr != log->consumed_at.data && log->consumed_at.size > 0) &&
        !::utils::datetime::is_valid_sqlite_datetime(
            wholth::utils::to_string_view(log->consumed_at)))
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_DATE, buffer);
    }

    if (nullptr != log->mass.data && log->mass.size > 0 &&
        !sqlw::utils::is_numeric(wholth::utils::to_string_view(log->mass)))
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_MASS, buffer);
    }

    std::array<wholth::utils::extended_param_t, 2> param_map{{
        {"consumed_at", log->consumed_at, sqlw::Type::SQL_TEXT, {}},
        {"mass", log->mass, sqlw::Type::SQL_DOUBLE, {}},
    }};

    std::stringstream ss;
    std::vector<sqlw::Statement::bindable_t> params{};
    params.reserve(3);
    ss << "UPDATE consumption_log SET ";
    wholth::utils::prepend_sql_params(param_map, params, ss);
    params.emplace_back(food_id, sqlw::Type::SQL_INT);
    params.emplace_back(id, sqlw::Type::SQL_INT);

    ss << fmt::format(" WHERE id = ?{0} ", params.size());

    const auto sql = ss.str();
    const std::error_code ec =
        sqlw::Transaction{&db::connection()}(sql, params);

    if (sqlw::status::Condition::OK != ec)
    {
        return push_and_get(ec, buffer);
    }

    return wholth_Error_OK;
}

wholth_Error wholth_em_consumption_log_delete(
    const wholth_ConsumptionLog* const log,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == log)
    {
        return push_and_get(Code::CONSUMPTION_LOG_NULL, buffer);
    }

    const auto id = wholth::utils::to_string_view(log->id);
    if (id.empty() || !is_valid_id(id))
    {
        return push_and_get(Code::CONSUMPTION_LOG_INVALID_ID, buffer);
    }

    const std::error_code ec = sqlw::Transaction{&db::connection()}(
        "DELETE FROM consumption_log WHERE id = ?1",
        std::array<sqlw::Statement::bindable_t, 1>{{
            {id, sqlw::Type::SQL_INT},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return push_and_get(ec, buffer);
    }

    return wholth_Error_OK;
}
