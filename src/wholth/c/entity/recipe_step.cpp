#include "wholth/c/entity/recipe_step.h"

extern "C" wholth_RecipeStep wholth_entity_recipe_step_init()
{
    return {
        .id = wholth_StringView_default,
        .time = wholth_StringView_default,
        .description = wholth_StringView_default,
        .ingredients_mass_g = wholth_StringView_default};
}
