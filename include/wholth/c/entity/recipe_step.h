#ifndef WHOLTH_C_ENTITY_RECIPE_STEP_H_
#define WHOLTH_C_ENTITY_RECIPE_STEP_H_

#include "wholth/c/forward.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct wholth_RecipeStep
    {
        wholth_StringView id;
        wholth_StringView time;
        wholth_StringView description;
    } wholth_RecipeStep;

    wholth_RecipeStep wholth_entity_recipe_step_init();

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_ENTITY_RECIPE_STEP_H_
