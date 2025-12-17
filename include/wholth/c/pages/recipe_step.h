#ifndef WHOTLH_C_PAGES_RECIPE_STEP_H_
#define WHOTLH_C_PAGES_RECIPE_STEP_H_

#include "wholth/c/entity/recipe_step.h"
#include "wholth/c/pages/utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // ARRAY_T(wholth_RecipeStep, wholth_RecipeStepArray);

    wholth_Error wholth_pages_recipe_step(wholth_Page**);
    // const wholth_RecipeStepArray wholth_pages_recipe_step_array(
    //     const wholth_Page* const);
    const wholth_RecipeStep* wholth_pages_recipe_step_first(
        const wholth_Page* const);
    void wholth_pages_recipe_step_recipe_id(
        wholth_Page* const,
        wholth_StringView recipe_id);

#ifdef __cplusplus
}
#endif

#endif // WHOTLH_C_PAGES_RECIPE_STEP_H_
