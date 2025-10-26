#ifndef WHOLTH_C_ERROR_H_
#define WHOLTH_C_ERROR_H_

#include "wholth/c/forward.h"
#include "wholth/c/string_view.h"
#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint64_t wholth_ErrorCode;

const wholth_ErrorCode WHOLTH_NO_ERROR = 0;

struct wholth_Error_t
{
    int64_t code;
    wholth_StringView message;
};
typedef struct wholth_Error_t wholth_Error;
const wholth_Error wholth_Error_OK = {0, {NULL, 0}};

bool wholth_error_ok(const wholth_Error* const err);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_ERROR_H_
