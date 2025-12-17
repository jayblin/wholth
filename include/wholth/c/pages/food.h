#ifndef WHOLTH_C_PAGES_FOOD_H_
#define WHOLTH_C_PAGES_FOOD_H_

#include "wholth/c/array.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif
    ARRAY_T(wholth_Food, wholth_FoodArray);
    typedef void* wholth_PageArray;

    wholth_Error wholth_pages_food(wholth_Page**, uint64_t per_page);
    // todo remove
    const wholth_FoodArray wholth_pages_food_array(const wholth_Page* const);
    void wholth_pages_food_id(
        wholth_Page* const,
        wholth_StringView search_id);
    void wholth_pages_food_title(
        wholth_Page* const,
        wholth_StringView search_title);
    void wholth_pages_food_ingredients(
        wholth_Page* const,
        wholth_StringView search_ingredients);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_PAGES_FOOD_H_
