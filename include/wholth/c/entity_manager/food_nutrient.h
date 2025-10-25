#ifndef WHOLTH_C_EM_FOOD_NUTRIENTS_H_
#define WHOLTH_C_EM_FOOD_NUTRIENTS_H_

#include "wholth/c/buffer.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/forward.h"
#include "wholth/c/entity/nutrient.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // const wholth_FoodNutrientArray wholth_food_nutrients();
    // void wholth_food_nutrients_food_id(const wholth_StringView food_id);
    // void wholth_food_nutrients_title(const wholth_StringView title);
    // void wholth_food_nutrients_fetch();
    // const wholth_Page wholth_food_nutrients_pagination();
    // bool wholth_food_nutrients_advance(uint64_t by);
    // bool wholth_food_nutrients_retreat(uint64_t by);
    // bool wholth_food_nutrients_skip_to(uint64_t page);

    wholth_Error wholth_em_food_nutrient_upsert(
        const wholth_Food* const,
        const wholth_Nutrient* const,
        wholth_Buffer* const);
    // wholth_Error wholth_em_update_food_nutrient(
    //     const wholth_Food*,
    //     const wholth_Nutrient*);
    wholth_Error wholth_em_food_nutrient_update_important(
        const wholth_Food* const,
        wholth_Buffer* const);

#ifdef __cplusplus
}
#endif

#endif // WHOLTH_C_EM_FOOD_NUTRIENTS_H_
