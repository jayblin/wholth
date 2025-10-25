#include "wholth/c/entity/ingredient.h"
#include "wholth/c/forward.h"

extern "C" wholth_Ingredient wholth_entity_ingredient_init()
{
    return {
        .id = wholth_StringView_default,
        .food_id = wholth_StringView_default,
        .food_title = wholth_StringView_default,
        .canonical_mass_g = wholth_StringView_default,
        .ingredient_count = wholth_StringView_default,
    };
}
