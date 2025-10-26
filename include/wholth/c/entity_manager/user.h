#ifndef WHOLTH_C_EM_USER_H_
#define WHOLTH_C_EM_USER_H_

#include "wholth/c/buffer.h"
#include "wholth/c/error.h"
#include "wholth/c/entity/user.h"

#ifdef __cplusplus
extern "C"
{
#endif

    wholth_Error wholth_em_user_authenticate(
        const wholth_User* const,
        const wholth_StringView password,
        wholth_Buffer* const);
    wholth_Error wholth_em_user_insert(
        wholth_User* const,
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
