#ifndef WHOLTH_C_NUTRIENT_H_
#define WHOLTH_C_NUTRIENT_H_

#include "wholth/c/forward.h"

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

    typedef struct wholth_Nutrient
    {
        wholth_StringView id;
        wholth_StringView title;
        wholth_StringView value;
        wholth_StringView unit;
        wholth_StringView position;
        /* struct wholth_StringView alias; */
        /* struct wholth_StringView description; */
    } wholth_Nutrient;

    wholth_Nutrient wholth_entity_nutrient_init();

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // WHOLTH_C_NUTRIENT_H_

