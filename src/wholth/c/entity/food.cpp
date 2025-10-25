#include "wholth/c/entity/food.h"
#include "wholth/c/forward.h"

wholth_Food wholth_entity_food_init()
{
    return {
        .id = wholth_StringView_default,
        .title = wholth_StringView_default,
        .preparation_time = wholth_StringView_default,
        .top_nutrient = wholth_StringView_default,
        .description = wholth_StringView_default,
    };
}
