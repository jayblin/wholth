#ifndef WHOLTH_APP_C_H_
#define WHOLTH_APP_C_H_

/* #include <cstdint> */
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

#define ARRAY_T(type, name) \
    struct name\
    {\
        const struct type* data;\
        const unsigned long size;\
    }

struct wholth_StringView
{
    const char* data;
    unsigned long size;
};

typedef struct wholth_StringView wholth_ErrorMessage;
typedef int wholth_ErrorCode;
const int WHOLTH_NO_ERROR = 0;

// todo rename
struct WholthContext
{
    const char* db_path;
};

struct wholth_Food
{
    struct wholth_StringView id;
    struct wholth_StringView title;
    struct wholth_StringView preparation_time;
    struct wholth_StringView top_nutrient;
};

struct wholth_FoodDetails
{
    struct wholth_StringView description;
};

struct wholth_Nutrient
{
    struct wholth_StringView id;
    struct wholth_StringView title;
    struct wholth_StringView value;
    struct wholth_StringView unit;
    struct wholth_StringView position;
    /* struct wholth_StringView alias; */
    /* struct wholth_StringView description; */
};

struct wholth_Ingredient
{
    struct wholth_StringView food;
    struct wholth_StringView canonical_mass;
    struct wholth_StringView ingredient_count;
};

struct wholth_RecipeStep
{
    struct wholth_StringView id;
    struct wholth_StringView time;
    struct wholth_StringView note;
    struct wholth_StringView description;
};

ARRAY_T(wholth_Food, wholth_FoodArray);
ARRAY_T(wholth_Nutrient, wholth_NutrientArray);
ARRAY_T(wholth_Ingredient, wholth_IngredientArray);
ARRAY_T(wholth_RecipeStep, wholth_RecipeStepArray);

// todo rename?
/* struct wholth_FoodsView */
/* { */
/*     const struct wholth_ShortenedFood* data; */
/*     unsigned long size; */
/* }; */

struct wholth_Page
{
    uint64_t max_page;
    uint64_t cur_page;
    struct wholth_StringView pagination;
};

wholth_ErrorMessage wholth_app_setup(const struct WholthContext);

bool wholth_foods_page_advance();
bool wholth_foods_page_retreat();
void wholth_foods_page_fetch();
bool wholth_foods_page_is_fetching();
struct wholth_Page wholth_foods_page_info();
// todo rename
const struct wholth_FoodArray wholth_foods_page_list();
void wholth_entity_food_fetch_nutrients(
    const struct wholth_StringView food_id
);
const struct wholth_NutrientArray wholth_entity_food_nutrients();
/* void wholth_entity_food_remove( */
/*     struct wholth_StringView id */
/* ); */
/* wholth_ErrorCode wholth_entity_food_insert( */
/*     struct wholth_StringView title, */
/*     struct wholth_StringView description */
/* ); */
/* wholth_ErrorCode wholth_entity_food_add_nutrients( */
/*     struct wholth_StringView food_id, */
/*     const int nutrient_count, */
/*     const struct wholth_StringView */
/* ); */
/* void wholth_entity_food_update( */
/*     struct wholth_StringView id, */
/*     struct wholth_StringView title, */
/*     struct wholth_StringView description */
/* ); */
void wholth_entity_food_detail(
    struct wholth_StringView food_id,
    struct wholth_FoodDetails* details
    /* struct wholth_Food* food */
    /* struct wholth_NutrientArray nutrients, */
    /* struct wholth_IngredientArray ingredients, */
    /* struct wholth_RecipeStepArray recipe_steps */
);
wholth_ErrorMessage wholth_latest_error_message();

/* const struct wholth_FoodsView internal_preview_wholth_foods(); */

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // WHOLTH_APP_C_H_
