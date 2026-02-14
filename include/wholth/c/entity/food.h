#ifndef WHOLTH_C_ENTITY_FOOD_H_
#define WHOLTH_C_ENTITY_FOOD_H_

#include "wholth/c/string_view.h"

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

    // struct wholth_Food;
    // const wholth_StringView wholth_entity_food_id(const wholth_Food* const);
    // const wholth_StringView wholth_entity_food_title(const wholth_Food* const);
    // const wholth_StringView wholth_entity_food_preparation_time(
    //     const wholth_Food* const);
    // const wholth_StringView wholth_entity_food_top_nutrient(
    //     const wholth_Food* const);
    // const wholth_StringView wholth_entity_food_top_description(
    //     const wholth_Food* const);

    typedef struct wholth_Food
    {
        wholth_StringView id;
        wholth_StringView title;
        //todo rename to smthsmth_seconds???
        wholth_StringView preparation_time;
        wholth_StringView top_nutrient;
        wholth_StringView ingredients_mass_g;
        wholth_StringView description;
    } wholth_Food;

    wholth_Food wholth_entity_food_init();

    // typedef struct wholth_FoodDetails
    // {
    //     wholth_StringView description;
    // } wholth_FoodDetails;

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // WHOLTH_C_ENTITY_FOOD_H_
