#include "wholth/c/entity_manager/user.h"
#include "db/db.hpp"
#include "sqlw/transaction.hpp"
#include "wholth/c/forward.h"
#include "wholth/c/internal.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_error.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <cassert>

using wholth::c::internal::push_and_get;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;
using wholth::utils::to_error;

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
        static constexpr std::string_view msg = "nullptr user";
        return to_error(601, msg);
    }

    const auto user_id = to_string_view(user->id);
    if (!is_valid_id(user_id))
    {
        static constexpr std::string_view msg = "bad user id";
        return to_error(602, msg);
    }

    const auto locale_id = to_string_view(user->locale_id);

    if (!is_valid_id(locale_id))
    {
        static constexpr std::string_view msg = "Bad locale id";
        return to_error(603, msg);
    }

    const auto ec = (sqlw::Transaction{&db::connection()})(
        "UPDATE user SET locale_id = ?1 WHERE id = ?2",
        std::array<sqlw::Statement::bindable_t, 2>{
            {{locale_id, sqlw::Type::SQL_INT},
             {user_id, sqlw::Type::SQL_INT}}});

    if (sqlw::status::Condition::OK != ec)
    {
        return push_and_get(ec, buffer);
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
