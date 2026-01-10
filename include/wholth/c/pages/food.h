#ifndef WHOLTH_C_PAGES_FOOD_H_
#define WHOLTH_C_PAGES_FOOD_H_

#include "wholth/c/array.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/error.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif
    enum wholth_pages_food_Code
    {
        _FOOD_PAGE_FIRST_ = 1000,
        FOOD_PAGE_TYPE_MISMATCH,
        FOOD_PAGE_BAD_LOCALE_ID,
        FOOD_PAGE_BAD_FOOD_ID,
        FOOD_PAGE_INGREDIENT_LIST_TOO_SHORT,
        FOOD_PAGE_TITLE_TOO_SHORT,
        _FOOD_PAGE_LAST_,
        _FOOD_PAGE_COUNT_ = _FOOD_PAGE_LAST_ - _FOOD_PAGE_FIRST_ - 1,
    };

    ARRAY_T(wholth_Food, wholth_FoodArray);
    typedef void* wholth_PageArray;

    wholth_Error wholth_pages_food(wholth_Page**, uint64_t per_page);
    /**
     * @deprecated todo remove
     */
    const wholth_FoodArray wholth_pages_food_array(const wholth_Page* const);
    wholth_Error wholth_pages_food_id(
        wholth_Page* const,
        wholth_StringView search_id);
    wholth_Error wholth_pages_food_locale_id(
        wholth_Page* const,
        wholth_StringView locale_id);
    wholth_Error wholth_pages_food_title(
        wholth_Page* const,
        wholth_StringView search_title);
    wholth_Error wholth_pages_food_ingredients(
        wholth_Page* const,
        wholth_StringView search_ingredients);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_PAGES_FOOD_H_
