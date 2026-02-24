#ifndef WHOLTH_C_PAGES_FOOD_NUTRIENT_H_
#define WHOLTH_C_PAGES_FOOD_NUTRIENT_H_

#include "wholth/c/array.h"
#include "wholth/c/error.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

    enum wholth_pages_food_nutrient_Code
    {
        wholth_pages_food_nutrient_Code_FIRST_ = 4000,
        wholth_pages_food_nutrient_Code_TYPE_MISMATCH,
        wholth_pages_food_nutrient_Code_BAD_LOCALE_ID,
        wholth_pages_food_nutrient_Code_BAD_FOOD_ID,
        wholth_pages_food_nutrient_Code_LAST_,
        wholth_pages_food_nutrient_Code_COUNT_ =
            wholth_pages_food_nutrient_Code_LAST_ -
            wholth_pages_food_nutrient_Code_FIRST_ - 1,
    };

    ARRAY_T(wholth_Nutrient, wholth_FoodNutrientArray);

    wholth_Error wholth_pages_food_nutrient(wholth_Page**, uint64_t per_page);
    // todo remove
    const wholth_FoodNutrientArray wholth_pages_food_nutrient_array(
        const wholth_Page* const);
    wholth_Error wholth_pages_food_nutrient_food_id(
        wholth_Page* const,
        wholth_StringView food_id);
    wholth_Error wholth_pages_food_nutrient_title(
        wholth_Page* const,
        wholth_StringView title);
    wholth_Error wholth_pages_food_nutrient_locale_id(
        wholth_Page* const,
        wholth_StringView locale_id);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_PAGES_FOOD_NUTRIENT_H_
