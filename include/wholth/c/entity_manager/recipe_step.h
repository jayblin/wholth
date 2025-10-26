#ifndef WHOLTH_C_EM_RECIPE_STEP_H_
#define WHOLTH_C_EM_RECIPE_STEP_H_

#include "wholth/c/buffer.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/entity/recipe_step.h"
#include "wholth/c/error.h"

#ifdef __cplusplus
extern "C"
{
#endif

    wholth_Error wholth_em_recipe_step_insert(
        wholth_RecipeStep* const,
        const wholth_Food* const,
        wholth_Buffer* const);
    wholth_Error wholth_em_recipe_step_update(
        const wholth_RecipeStep* const,
        wholth_Buffer* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_EM_RECIPE_STEP_H_
