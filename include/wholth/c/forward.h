#ifndef WHOLTH_C_FORWARD_H_
#define WHOLTH_C_FORWARD_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */

#include <cstddef>
extern "C" {

#endif

#ifndef NULL
#define NULL 0
#endif

#define ARRAY_T(type, name) \
    typedef struct name\
    {\
        const struct type* const data;\
        const unsigned long size;\
    } name

struct wholth_StringView_t
{
    const char* data;
    unsigned long size;
} const wholth_StringView_default = {NULL, 0};
typedef struct wholth_StringView_t wholth_StringView;

struct wholth_Error_t
{
    int code;
    wholth_StringView message;
};
typedef struct wholth_Error_t wholth_Error;
const wholth_Error wholth_Error_OK = {0, {NULL, 0}};

bool wholth_ok(const wholth_Error* const err);

typedef wholth_StringView wholth_ErrorMessage;
typedef uint64_t wholth_ErrorCode;

const wholth_ErrorCode WHOLTH_NO_ERROR = 0;

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // WHOLTH_C_FORWARD_H_
