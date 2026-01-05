#ifndef WHOLTH_C_EM_USER_H_
#define WHOLTH_C_EM_USER_H_

#include "wholth/c/buffer.h"
#include "wholth/c/error.h"
#include "wholth/c/entity/user.h"

#ifdef __cplusplus
extern "C"
{
#endif

    enum wholth_em_user_Code
    {
        _USER_FIRST_ = 200,
        USER_NULL,
        USER_INVALID_ID,
        USER_INVALID_LOCALE_ID,
        USER_AUTHENTICATION_FAILED,
        USER_AUTHENTICATION_FAILED_DRAGOON,
        USER_AUTHENTICATION_FAILED_CHOOMBA,
        USER_AUTHENTICATION_FAILED_CHROME,
        USER_AUTHENTICATION_FAILED_PREEM,
        USER_AUTHENTICATION_FAILED_KLEPPED,
        USER_UNAUTHORIZED,
        USER_PASSWORD_TOO_SHORT,
        USER_PASSWORD_TOO_LONG,
        USER_NO_NAME,
        USER_AUTHENTICATION_FAILED_GONKED,
        USER_DOES_NOT_EXIST,
        _USER_LAST_,
        _USER_COUNT_ = _USER_LAST_ - _USER_FIRST_ - 1,
    };

    wholth_Error wholth_em_user_exists(
        const wholth_StringView name,
        wholth_StringView* const id,
        wholth_Buffer* const);
    wholth_Error wholth_em_user_authenticate(
        wholth_User* const,
        const wholth_StringView password,
        wholth_Buffer* const);
    wholth_Error wholth_em_user_insert(
        wholth_User* const,
        const wholth_StringView password,
        wholth_Buffer* const);
    wholth_Error wholth_em_user_update(
        const wholth_User* const,
        wholth_Buffer* const);
    wholth_Error wholth_em_user_locale_id(
        const wholth_StringView user_id,
        const wholth_StringView locale_id,
        wholth_Buffer* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_EM_USER_H_
