#ifndef WHOLTH_C_ENTITY_USER_H_
#define WHOLTH_C_ENTITY_USER_H_

#include "wholth/c/string_view.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct wholth_User
    {
        wholth_StringView id;
        wholth_StringView name;
        wholth_StringView locale_id;
    } wholth_RecipeStep;

    wholth_RecipeStep wholth_entity_user_init();

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_ENTITY_USER_H_
