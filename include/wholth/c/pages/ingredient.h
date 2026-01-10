#ifndef WHOTLH_C_PAGES_INGREDIENT_H_
#define WHOTLH_C_PAGES_INGREDIENT_H_

#include "wholth/c/array.h"
#include "wholth/c/entity/ingredient.h"
#include "wholth/c/error.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif
    enum wholth_pages_ingredient_Code
    {
        _INGREDIENT_PAGE_FIRST_ = 2000,
        INGREDIENT_PAGE_TYPE_MISMATCH,
        INGREDIENT_PAGE_BAD_LOCALE_ID,
        INGREDIENT_PAGE_TITLE_TOO_SHORT,
        INGREDIENT_PAGE_BAD_FOOD_ID,
        _INGREDIENT_PAGE_LAST_,
        _INGREDIENT_PAGE_COUNT_ = _INGREDIENT_PAGE_LAST_ - _INGREDIENT_PAGE_FIRST_ - 1,
    };

    ARRAY_T(wholth_Ingredient, wholth_IngredientArray);

    wholth_Error wholth_pages_ingredient(wholth_Page**, uint64_t per_page);
    // todo remove
    const wholth_IngredientArray wholth_pages_ingredient_array(
        const wholth_Page* const);
    wholth_Error wholth_pages_ingredient_food_id(
        wholth_Page* const,
        wholth_StringView food_id);
    wholth_Error wholth_pages_ingredient_title(
        wholth_Page* const,
        wholth_StringView title);
    wholth_Error wholth_pages_ingredient_locale_id(
        wholth_Page* const,
        wholth_StringView title);

#ifdef __cplusplus
}
#endif

#endif // WHOTLH_C_PAGES_INGREDIENT_H_
