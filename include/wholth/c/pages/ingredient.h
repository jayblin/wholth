#ifndef WHOTLH_C_PAGES_INGREDIENT_H_
#define WHOTLH_C_PAGES_INGREDIENT_H_

#include "wholth/c/entity/ingredient.h"
#include "wholth/c/forward.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ARRAY_T(wholth_Ingredient, wholth_IngredientArray);

    wholth_Page* wholth_pages_ingredient(uint64_t per_page, bool reset);
    const wholth_IngredientArray wholth_pages_ingredient_array(
        const wholth_Page* const);
    void wholth_pages_ingredient_food_id(
        wholth_Page* const,
        wholth_StringView food_id);
    void wholth_pages_ingredient_title(
        wholth_Page* const,
        wholth_StringView title);

#ifdef __cplusplus
}
#endif

#endif // WHOTLH_C_PAGES_INGREDIENT_H_
