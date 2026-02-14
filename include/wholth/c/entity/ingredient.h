#ifndef WHOLTH_C_ENTITY_INGREDIENTS_H_
#define WHOLTH_C_ENTITY_INGREDIENTS_H_

#include "wholth/c/string_view.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct wholth_Ingredient
    {
        wholth_StringView id;
        wholth_StringView food_id;
        wholth_StringView food_title;
        wholth_StringView canonical_mass_g;
        wholth_StringView ingredient_count;
        wholth_StringView ingredients_mass_g;
    } wholth_Ingredient;

    wholth_Ingredient wholth_entity_ingredient_init();

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_ENTITY_INGREDIENTS_H_
