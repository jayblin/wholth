#ifndef WHOLTH_C_STRING_VIEW_H_
#define WHOLTH_C_STRING_VIEW_H_

#include "wholth/c/array.h"
#include "wholth/c/forward.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct wholth_StringView_t
    {
        const char* data;
        unsigned long size;
    };
    typedef struct wholth_StringView_t wholth_StringView;
    extern const wholth_StringView wholth_StringView_default;

    // wholth_StringView wholth_default_string_view();

    ARRAY_MUTABLE_T(wholth_StringView_t, wholth_StringViewArrayMutable);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_STRING_VIEW_H_
