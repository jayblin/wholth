#include "wholth/error.hpp"
#include "wholth/c/pages/food.h"
#include "wholth/c/pages/food_nutrient.h"
#include "wholth/c/pages/nutrient.h"
#include "wholth/c/pages/ingredient.h"
#include "wholth/c/entity_manager/food.h"
#include "wholth/c/exec_stmt.h"
#include <array>
#include <cstddef>
#include <string_view>
#include <tuple>
#include <utility>

const char* wholth::error::Category::name() const noexcept
{
    return "wholth";
}

#define FIRST_AND_LAST(ns)                                                     \
    {static_cast<size_t>(wholth_##ns##_Code_FIRST_),                           \
     static_cast<size_t>(wholth_##ns##_Code_LAST_)}

constexpr std::array<std::pair<size_t, size_t>, 6> registered_codes = {{
    FIRST_AND_LAST(em_food),
    FIRST_AND_LAST(pages_food),
    FIRST_AND_LAST(pages_nutrient),
    FIRST_AND_LAST(pages_food_nutrient),
    FIRST_AND_LAST(pages_ingredient),
    FIRST_AND_LAST(exec_stmt),
}};

constexpr bool does_intersect(const size_t& idx)
{
    bool result = false;
    std::apply(
        [&result, &idx](const auto&... pairs) {
            size_t i = 0;
            ((result = (result == true || i == idx)
                           ? result
                           : pairs.first == registered_codes[idx].first,
              i++),
             ...);
        },
        registered_codes);

    return result;
}

constexpr bool do_error_codes_overlap()
{
    bool result = false;
    std::apply(
        [&result](const auto&... pairs) {
            size_t i = 0;
            ((result = (result == true) ? result : does_intersect(i),
              i++,
              (void)pairs),
             ...);
        },
        registered_codes);

    return result;
}

// todo переделать на русский.
std::string_view wholth::error::message(int ev)
{
    static_assert(false == do_error_codes_overlap());

    if (wholth_pages_ingredient_Code::wholth_pages_ingredient_Code_FIRST_ < ev &&
        wholth_pages_ingredient_Code::wholth_pages_ingredient_Code_LAST_ > ev)
    {
        switch (static_cast<wholth_pages_ingredient_Code>(ev))
        {
        case wholth_pages_ingredient_Code_BAD_LOCALE_ID:
            return "Bad locale id provided for ingredient managpageent "
                   "function!";
        case wholth_pages_ingredient_Code_FIRST_:
        case wholth_pages_ingredient_Code_LAST_:
        case wholth_pages_ingredient_Code_COUNT_:
            return "Not an error.";
        case wholth_pages_ingredient_Code_TYPE_MISMATCH:
            return "You are expected to provide a pointer to a page of "
                   "Ingredients!";
        case wholth_pages_ingredient_Code_TITLE_TOO_SHORT:
            return "Ingredient's title should be at least 3 chars long!";
        case wholth_pages_ingredient_Code_BAD_FOOD_ID:
            return "Provided bad ingredient id!";
        }
    }

    if (wholth_em_food_Code::wholth_em_food_Code_FIRST_ < ev &&
        wholth_em_food_Code::wholth_em_food_Code_LAST_ > ev)
    {
        switch (static_cast<wholth_em_food_Code>(ev))
        {
        case wholth_em_food_Code_BAD_LOCALE_ID:
            return "Bad locale id provided for food management function!";
        case wholth_em_food_Code_DUPLICATE_ENTRY:
            return "Food with same title already exists for chosen locale!";
        case wholth_em_food_Code_FIRST_:
        case wholth_em_food_Code_LAST_:
        case wholth_em_food_Code_COUNT_:
            return "Not an error.";
        }
    }

    if (wholth_pages_food_Code::wholth_pages_food_Code_FIRST_ < ev &&
        wholth_pages_food_Code::wholth_pages_food_Code_LAST_ > ev)
    {
        switch (static_cast<wholth_pages_food_Code>(ev))
        {
        case wholth_pages_food_Code_TYPE_MISMATCH:
            return "You are expected to provide a pointer to a page of "
                   "Foods!";
        case wholth_pages_food_Code_BAD_LOCALE_ID:
            return "Bad locale id provided for food page query!";
        case wholth_pages_food_Code_BAD_FOOD_ID:
            return "Bad food id provided for food page query!";
        case wholth_pages_food_Code_INGREDIENT_LIST_TOO_SHORT:
            return "Ingredient list is too short. Needs to be at least 3 "
                   "chars!";
        case wholth_pages_food_Code_FIRST_:
        case wholth_pages_food_Code_LAST_:
        case wholth_pages_food_Code_COUNT_:
            return "Not an error.";
        case wholth_pages_food_Code_TITLE_TOO_SHORT:
            return "Title you are searching for is too short. Needs to be at "
                   "least 3 chars!";
            break;
        }
    }

    if (wholth_pages_nutrient_Code::wholth_pages_nutrient_Code_FIRST_ < ev &&
        wholth_pages_nutrient_Code::wholth_pages_nutrient_Code_LAST_ > ev)
    {
        switch (static_cast<wholth_pages_nutrient_Code>(ev))
        {
        case wholth_pages_nutrient_Code_TYPE_MISMATCH:
            return "You are expected to provide a pointer to a page of "
                   "Nutrients!";
        case wholth_pages_nutrient_Code_BAD_LOCALE_ID:
            return "Bad locale id provided for nutrient page query!";
        case wholth_pages_nutrient_Code_FIRST_:
        case wholth_pages_nutrient_Code_LAST_:
        case wholth_pages_nutrient_Code_COUNT_:
            return "Not an error.";
        }
    }

    if (wholth_pages_food_nutrient_Code::wholth_pages_food_nutrient_Code_FIRST_ < ev &&
        wholth_pages_food_nutrient_Code::wholth_pages_food_nutrient_Code_LAST_ > ev)
    {
        switch (static_cast<wholth_pages_food_nutrient_Code>(ev))
        {
        case wholth_pages_food_nutrient_Code_TYPE_MISMATCH:
            return "You are expected to provide a pointer to a page of "
                   "Nutrients assigned to a speific food!";
        case wholth_pages_food_nutrient_Code_BAD_LOCALE_ID:
            return "Bad locale id provided for food nutrient page query!";
        case wholth_pages_food_nutrient_Code_BAD_FOOD_ID:
            return "Bad food id provided for food nutrient page query!";
        case wholth_pages_food_nutrient_Code_FIRST_:
        case wholth_pages_food_nutrient_Code_LAST_:
        case wholth_pages_food_nutrient_Code_COUNT_:
            return "Not an error.";
        }
    }

    return "(unrecognized error)";
}

std::string wholth::error::Category::message(int ev) const
{
    return std::string{wholth::error::message(ev)};
}

const wholth::error::Category& wholth::error::category()
{
    static const Category category{};

    return category;
}

extern "C" const wholth_ErrorCode WHOLTH_NO_ERROR = 0;
extern "C" const wholth_Error     wholth_Error_OK = {0, {NULL, 0}};
