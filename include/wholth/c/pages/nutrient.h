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
        wholth_pages_nutrient_Code_FIRST_ = 3000,
        wholth_pages_nutrient_Code_TYPE_MISMATCH,
        wholth_pages_nutrient_Code_BAD_LOCALE_ID,
        wholth_pages_nutrient_Code_LAST_,
        wholth_pages_nutrient_Code_COUNT_ = wholth_pages_nutrient_Code_LAST_ -
                                            wholth_pages_nutrient_Code_FIRST_ -
                                            1,
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
