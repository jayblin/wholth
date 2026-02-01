#ifndef WHOLTH_C_ERROR_H_
#define WHOLTH_C_ERROR_H_

#include "wholth/c/string_view.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef unsigned long long wholth_ErrorCode;

    extern const wholth_ErrorCode WHOLTH_NO_ERROR;

    struct wholth_Error_t
    {
        wholth_ErrorCode code;
        wholth_StringView message;
    };
    typedef struct wholth_Error_t wholth_Error;

    extern const wholth_Error wholth_Error_OK;

    bool wholth_error_ok(const wholth_Error* const err);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_ERROR_H_
