#include "wholth/c/entity/nutrient.h"
#include "wholth/c/forward.h"

extern "C" wholth_Nutrient wholth_entity_nutrient_init()
{
    return {
        .id = wholth_StringView_default,
        .title = wholth_StringView_default,
        .value = wholth_StringView_default,
        .unit = wholth_StringView_default,
        .position = wholth_StringView_default,
    };
}
