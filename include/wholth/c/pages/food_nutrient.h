#ifndef WHOLTH_C_PAGES_FOOD_NUTRIENT_H_
#define WHOLTH_C_PAGES_FOOD_NUTRIENT_H_

#include "wholth/c/array.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ARRAY_T(wholth_Nutrient, wholth_FoodNutrientArray);

    wholth_Error wholth_pages_food_nutrient(wholth_Page**, uint64_t per_page);
    // todo remove
    const wholth_FoodNutrientArray wholth_pages_food_nutrient_array(
        const wholth_Page* const);
    void wholth_pages_food_nutrient_food_id(
        wholth_Page* const,
        wholth_StringView food_id);
    void wholth_pages_food_nutrient_title(
        wholth_Page* const,
        wholth_StringView title);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_PAGES_FOOD_NUTRIENT_H_
