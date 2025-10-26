#ifndef WHOLTH_C_EM_INGREDIENT_H_
#define WHOLTH_C_EM_INGREDIENT_H_

#include "wholth/c/buffer.h"
#include "wholth/c/entity/ingredient.h"
#include "wholth/c/entity/recipe_step.h"
#include "wholth/c/error.h"

#ifdef __cplusplus
extern "C"
{
#endif

    wholth_Error wholth_em_ingredient_insert(
        wholth_Ingredient* const,
        const wholth_RecipeStep* const, wholth_Buffer* const);
    wholth_Error wholth_em_ingredient_update(
        const wholth_Ingredient* const,
        const wholth_RecipeStep* const, wholth_Buffer* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_EM_INGREDIENT_H_
