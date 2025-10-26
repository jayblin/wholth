#ifndef WHOTLH_C_PAGES_NUTRIENT_H_
#define WHOTLH_C_PAGES_NUTRIENT_H_

#include "wholth/c/array.h"
#include "wholth/c/entity/nutrient.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ARRAY_T(wholth_Nutrient, wholth_NutrientArray);

    wholth_Page* wholth_pages_nutrient(uint64_t per_page, bool reset);
    const wholth_NutrientArray wholth_pages_nutrient_array(
        const wholth_Page* const);
    void wholth_pages_nutrient_title(
        wholth_Page* const,
        wholth_StringView title);

#ifdef __cplusplus
}
#endif

#endif // WHOTLH_C_PAGES_NUTRIENT_H_
