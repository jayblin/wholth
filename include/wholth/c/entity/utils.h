#ifndef WHOLTH_C_ENTITY_UTILS_H_
#define WHOLTH_C_ENTITY_UTILS_H_

#include "wholth/c/string_view.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct wholth_Entity_t;
    typedef struct wholth_Entity_t wholth_Entity;

   wholth_StringViewArrayMutable wholth_entity_values(const wholth_Entity* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_ENTITY_UTILS_H_
