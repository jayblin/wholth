#ifndef WHOTLH_C_PAGES_NUTRIENT_H_
#define WHOTLH_C_PAGES_NUTRIENT_H_

#include "wholth/c/array.h"
#include "wholth/c/entity/nutrient.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif
    enum wholth_pages_nutrient_Code
    {
        _NUTRIENT_PAGE_FIRST_ = 3000,
        NUTRIENT_PAGE_TYPE_MISMATCH,
        NUTRIENT_PAGE_BAD_LOCALE_ID,
        _NUTRIENT_PAGE_LAST_,
        _NUTRIENT_PAGE_COUNT_ = _NUTRIENT_PAGE_LAST_ - _NUTRIENT_PAGE_FIRST_ - 1,
    };

    ARRAY_T(wholth_Nutrient, wholth_NutrientArray);

    wholth_Error wholth_pages_nutrient(wholth_Page**, uint64_t per_page);
    // todo remove
    const wholth_NutrientArray wholth_pages_nutrient_array(
        const wholth_Page* const);
    wholth_Error wholth_pages_nutrient_title(
        wholth_Page* const,
        wholth_StringView title);
    wholth_Error wholth_pages_nutrient_locale_id(
        wholth_Page* const,
        wholth_StringView locale_id);

#ifdef __cplusplus
}
#endif

#endif // WHOTLH_C_PAGES_NUTRIENT_H_
