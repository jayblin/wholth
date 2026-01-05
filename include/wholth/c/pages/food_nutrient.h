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
        _FOOD_NUTRIENT_PAGE_FIRST_ = 4000,
        FOOD_NUTRIENT_PAGE_TYPE_MISMATCH,
        FOOD_NUTRIENT_PAGE_BAD_LOCALE_ID,
        FOOD_NUTRIENT_PAGE_BAD_FOOD_ID,
        _FOOD_NUTRIENT_PAGE_LAST_,
        _FOOD_NUTRIENT_PAGE_COUNT_ =
            _FOOD_NUTRIENT_PAGE_LAST_ - _FOOD_NUTRIENT_PAGE_FIRST_ - 1,
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
