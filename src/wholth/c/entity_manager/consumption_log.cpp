#include "db/db.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "sqlw/utils.hpp"
#include "utils/datetime.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity/consumption_log.h"
#include "wholth/c/entity_manager/consumption_log.h"
#include "wholth/c/internal.hpp"
#include "wholth/entity_manager/consumption_log.hpp"
#include "wholth/entity_manager/user.hpp"
#include "wholth/entity_manager/user.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/prepend_sql_params.hpp"
#include "wholth/utils/to_string_view.hpp"

using ::utils::datetime::is_valid_sqlite_datetime;
using wholth::c::internal::ec_to_error;
using wholth::entity_manager::consumption_log::Code;
using wholth::utils::current_time_and_date;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;

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
        switch (static_cast<Code>(ev))
        {
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

std::error_code make_error_code(wholth::entity_manager::consumption_log::Code e)
{
    return {
        static_cast<int>(e),
        wholth::entity_manager::consumption_log::error_category};
}

extern "C" wholth_Error wholth_em_consumption_log_insert(
    wholth_ConsumptionLog* const log,
    wholth_User* const user,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    std::error_code err;
    std::string_view user_id;
    std::string_view food_id;
    std::string_view mass;
    std::string_view datetime;

    if (nullptr == log)
    {
        err = Code::CONSUMPTION_LOG_NULL;
    }
    else if (nullptr == user)
    {
        err = wholth::entity_manager::user::Code::USER_NULL;
    }
    else if (user_id = to_string_view(user->id); !is_valid_id(user_id))
    {
        err = wholth::entity_manager::user::Code::USER_INVALID_ID;
    }
    else if (food_id = to_string_view(log->food_id); !is_valid_id(food_id))
    {
        err = Code::CONSUMPTION_LOG_INVALID_FOOD_ID;
    }
    else if (mass = to_string_view(log->mass);
             mass.empty() || !sqlw::utils::is_numeric(mass))
    {
        err = Code::CONSUMPTION_LOG_INVALID_MASS;
    }
    else if (datetime = to_string_view(log->consumed_at);
             datetime.empty() ||
             !::utils::datetime::is_valid_sqlite_datetime(datetime))
    {
        err = Code::CONSUMPTION_LOG_INVALID_DATE;
    }

    if (err)
    {
        return ec_to_error(err, buffer);
    }

    std::string id;
    const std::error_code ec = sqlw::Transaction{&db::connection()}(
        "INSERT INTO consumption_log (food_id, user_id, mass, consumed_at) "
        "VALUES (?1, ?2, ?3, ?4) RETURNING id",
        [&id](sqlw::Statement::ExecArgs e) { id = e.column_value; },
        std::array<sqlw::Statement::bindable_t, 4>{{
            {food_id, sqlw::Type::SQL_INT},
            {user_id, sqlw::Type::SQL_INT},
            {mass, sqlw::Type::SQL_DOUBLE},
            {datetime, sqlw::Type::SQL_TEXT},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    if (id.empty())
    {
        return ec_to_error(Code::CONSUMPTION_LOG_INVALID_ID, buffer);
    }

    wholth_buffer_move_data_to(buffer, &id);
    log->id = wholth_buffer_view(buffer);

    return wholth_Error_OK;
}

extern "C" wholth_Error wholth_em_consumption_log_update(
    const wholth_ConsumptionLog* const log,
    wholth_User* const user,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    std::error_code err;
    std::string_view id;
    std::string_view user_id;
    std::string_view food_id;

    if (nullptr == log)
    {
        err = Code::CONSUMPTION_LOG_NULL;
    }
    else if (nullptr == user)
    {
        err = wholth::entity_manager::user::Code::USER_NULL;
    }
    else if (id = to_string_view(log->id); !is_valid_id(id))
    {
        err = Code::CONSUMPTION_LOG_INVALID_ID;
    }
    else if (user_id = to_string_view(user->id); !is_valid_id(user_id))
    {
        err = wholth::entity_manager::user::Code::USER_INVALID_ID;
    }
    else if (food_id = to_string_view(log->food_id); !is_valid_id(food_id))
    {
        err = Code::CONSUMPTION_LOG_INVALID_FOOD_ID;
    }
    else if (
        (nullptr != log->consumed_at.data && log->consumed_at.size > 0) &&
        !is_valid_sqlite_datetime(to_string_view(log->consumed_at)))
    {
        err = Code::CONSUMPTION_LOG_INVALID_DATE;
    }
    else if (
        nullptr != log->mass.data && log->mass.size > 0 &&
        !sqlw::utils::is_numeric(to_string_view(log->mass)))
    {
        err = Code::CONSUMPTION_LOG_INVALID_MASS;
    }

    if (err)
    {
        return ec_to_error(err, buffer);
    }

    std::array<wholth::utils::extended_param_t, 2> param_map{{
        {"consumed_at", log->consumed_at, sqlw::Type::SQL_TEXT, {}},
        {"mass", log->mass, sqlw::Type::SQL_DOUBLE, {}},
    }};

    std::stringstream ss;
    std::vector<sqlw::Statement::bindable_t> params{};
    params.reserve(4);
    ss << "UPDATE consumption_log SET ";
    wholth::utils::prepend_sql_params(param_map, params, ss);
    params.emplace_back(id, sqlw::Type::SQL_INT);
    params.emplace_back(user_id, sqlw::Type::SQL_INT);

    ss << fmt::format(
        " WHERE id = ?{0} AND user_id = ?{1}",
        params.size() - 1,
        params.size());

    const auto sql = ss.str();
    const std::error_code ec =
        sqlw::Transaction{&db::connection()}(sql, params);

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}

wholth_Error wholth_em_consumption_log_delete(
    const wholth_ConsumptionLog* const log,
    wholth_User* const user,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    std::error_code err;

    std::string_view id;
    std::string_view user_id;

    if (nullptr == log)
    {
        err = Code::CONSUMPTION_LOG_NULL;
    }
    else if (nullptr == user)
    {
        err = wholth::entity_manager::user::Code::USER_NULL;
    }
    else if (user_id = to_string_view(user->id); !is_valid_id(user_id))
    {
        err = wholth::entity_manager::user::Code::USER_INVALID_ID;
    }
    else if (id = to_string_view(log->id); !is_valid_id(id))
    {
        err = Code::CONSUMPTION_LOG_INVALID_ID;
    }

    if (err)
    {
        return ec_to_error(err, buffer);
    }

    const std::error_code ec = sqlw::Transaction{&db::connection()}(
        "DELETE FROM consumption_log WHERE id = ?1 AND user_id = ?2",
        std::array<sqlw::Statement::bindable_t, 2>{{
            {id, sqlw::Type::SQL_INT},
            {user_id, sqlw::Type::SQL_INT},
        }});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}
