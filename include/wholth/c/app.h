#ifndef WHOLTH_C_APP_H
#define WHOLTH_C_APP_H

#include <stdbool.h>

#include "wholth/c/error.h"
#include "wholth/c/string_view.h"

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

    typedef struct wholth_AppSetupArgs
    {
        // todo change db_path to wholth_StringView ???
        const char* db_path;
    } wholth_AppSetupArgs;

    wholth_Error wholth_app_setup(const wholth_AppSetupArgs* const);

    // todo rename
    void wholth_user_locale_id(const wholth_StringView);
    void wholth_app_locale_id(const wholth_StringView);

    void wholth_app_password_encryption_secret(wholth_StringView secret);

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // WHOLTH_C_APP_H

