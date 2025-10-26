#include "wholth/c/entity_manager/user.h"
#include "db/db.hpp"
#include "sqlw/transaction.hpp"
#include "wholth/c/internal.hpp"
#include "wholth/entity_manager/user.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <cassert>

using wholth::c::internal::ec_to_error;
using wholth::entity_manager::user::Code;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;

namespace wholth::entity_manager::user
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "entity_manager::user";
    }

    std::string message(int ev) const override final
    {
        switch (static_cast<Code>(ev))
        {
        case USER_NULL:
            return "USER_NULL";
        case USER_INVALID_ID:
            return "USER_INVALID_ID";
        case USER_INVALID_LOCALE_ID:
            return "USER_INVALID_LOCALE_ID";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory error_category{};
} // namespace wholth::entity_manager::user

std::error_code make_error_code(wholth::entity_manager::user::Code e)
{
    return {static_cast<int>(e), wholth::entity_manager::user::error_category};
}

extern "C" wholth_Error wholth_em_user_update(
    const wholth_User* const user,
    wholth_Buffer* const buffer)
{
    if (nullptr == buffer)
    {
        return wholth::c::internal::bad_buffer_error();
    }

    if (nullptr == user)
    {
        return ec_to_error(Code::USER_NULL, buffer);
    }

    const auto user_id = to_string_view(user->id);
    if (!is_valid_id(user_id))
    {
        return ec_to_error(Code::USER_INVALID_ID, buffer);
    }

    const auto locale_id = to_string_view(user->locale_id);

    if (!is_valid_id(locale_id))
    {
        return ec_to_error(Code::USER_INVALID_LOCALE_ID, buffer);
    }

    const auto ec = (sqlw::Transaction{&db::connection()})(
        "UPDATE user SET locale_id = ?1 WHERE id = ?2",
        std::array<sqlw::Statement::bindable_t, 2>{
            {{locale_id, sqlw::Type::SQL_INT},
             {user_id, sqlw::Type::SQL_INT}}});

    if (sqlw::status::Condition::OK != ec)
    {
        return ec_to_error(ec, buffer);
    }

    return wholth_Error_OK;
}

extern "C" wholth_Error wholth_em_user_locale_id(
    const wholth_StringView user_id,
    const wholth_StringView locale_id,
    wholth_Buffer* const buffer)
{
    const wholth_User user{
        .id = user_id,
        .locale_id = locale_id,
    };
    return wholth_em_user_update(&user, buffer);
}
