#ifndef WHOLTH_APP_C_H_
#define WHOLTH_APP_C_H_

/* #include <cstdint> */
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

#define ARRAY_T(type, name) \
    typedef struct name\
    {\
        const struct type* data;\
        const unsigned long size;\
    } name

typedef struct wholth_StringView
{
    const char* data;
    unsigned long size;
} wholth_StringView;

typedef struct wholth_StringView wholth_ErrorMessage;
typedef int wholth_ErrorCode;
const int WHOLTH_NO_ERROR = 0;

// todo rename
typedef struct WholthContext
{
    const char* db_path;
} WholthContext;

typedef struct wholth_Food
{
    struct wholth_StringView id;
    struct wholth_StringView title;
    struct wholth_StringView preparation_time;
    struct wholth_StringView top_nutrient;
} wholth_Food;

typedef struct wholth_FoodDetails
{
    struct wholth_StringView description;
} wholth_FoodDetails;

typedef struct wholth_Nutrient
{
    struct wholth_StringView id;
    struct wholth_StringView title;
    struct wholth_StringView value;
    struct wholth_StringView unit;
    struct wholth_StringView position;
    /* struct wholth_StringView alias; */
    /* struct wholth_StringView description; */
} wholth_Nutrient;

typedef struct wholth_Ingredient
{
    struct wholth_StringView food;
    struct wholth_StringView canonical_mass;
    struct wholth_StringView ingredient_count;
} wholth_Ingredient;

typedef struct wholth_RecipeStep
{
    struct wholth_StringView id;
    struct wholth_StringView time;
    struct wholth_StringView note;
    struct wholth_StringView description;
} wholth_RecipeStep;

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

// todo rename
typedef struct wholth_Page
{
    uint64_t max_page;
    uint64_t cur_page;
    /* struct wholth_StringView pagination; // not used */
} wholth_Page;

wholth_ErrorMessage wholth_app_setup(const WholthContext);

bool wholth_foods_page_advance(uint64_t by);
bool wholth_foods_page_retreat(uint64_t by);
bool wholth_foods_page_skip_to(uint64_t page);
void wholth_foods_page_fetch();
void wholth_foods_page_title(wholth_StringView search_title);
bool wholth_foods_page_is_fetching();
// todo rename
wholth_Page wholth_foods_page_info();
// todo rename
const wholth_FoodArray wholth_foods_page_list();

const wholth_NutrientArray wholth_food_nutrients();
void wholth_food_nutrients_food_id(const wholth_StringView food_id);
void wholth_food_nutrients_title(const wholth_StringView title);
void wholth_food_nutrients_fetch();
const wholth_Page wholth_food_nutrients_pagination();
bool wholth_food_nutrients_advance(uint64_t by);
bool wholth_food_nutrients_retreat(uint64_t by);
bool wholth_food_nutrients_skip_to(uint64_t page);

const wholth_FoodDetails wholth_food_details();
void wholth_food_details_food_id(
    const wholth_StringView food_id
);
void wholth_food_details_fetch();

wholth_ErrorMessage wholth_latest_error_message();

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // WHOLTH_APP_C_H_
