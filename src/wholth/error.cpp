#include "wholth/error.hpp"
#include "wholth/c/pages/food.h"
#include "wholth/c/pages/food_nutrient.h"
#include "wholth/c/pages/nutrient.h"
#include "wholth/c/pages/ingredient.h"
#include "wholth/c/entity_manager/food.h"
#include <array>
#include <cstddef>
#include <string_view>
#include <tuple>
#include <utility>

const char* wholth::error::Category::name() const noexcept
{
    return "wholth";
}

#define FIRST_AND_LAST(ns, prefix)                                             \
    {static_cast<size_t>(wholth_##ns##_Code ::prefix##_FIRST_),                \
     static_cast<size_t>(wholth_##ns##_Code ::prefix##_LAST_)}

constexpr std::array<std::pair<size_t, size_t>, 5> registered_codes = {{
    FIRST_AND_LAST(em_food, _FOOD_EM),
    FIRST_AND_LAST(pages_food, _FOOD_PAGE),
    FIRST_AND_LAST(pages_nutrient, _NUTRIENT_PAGE),
    FIRST_AND_LAST(pages_food_nutrient, _FOOD_NUTRIENT_PAGE),
    FIRST_AND_LAST(pages_ingredient, _INGREDIENT_PAGE),
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

    if (wholth_pages_ingredient_Code::_INGREDIENT_PAGE_FIRST_ < ev &&
        wholth_pages_ingredient_Code::_INGREDIENT_PAGE_LAST_ > ev)
    {
        switch (static_cast<wholth_pages_ingredient_Code>(ev))
        {
        case INGREDIENT_PAGE_BAD_LOCALE_ID:
            return "Bad locale id provided for ingredient managpageent "
                   "function!";
        case _INGREDIENT_PAGE_FIRST_:
        case _INGREDIENT_PAGE_LAST_:
        case _INGREDIENT_PAGE_COUNT_:
            return "Not an error.";
        case INGREDIENT_PAGE_TYPE_MISMATCH:
            return "You are expected to provide a pointer to a page of "
                   "Ingredients!";
        case INGREDIENT_PAGE_TITLE_TOO_SHORT:
            return "Ingredient's title should be at least 3 chars long!";
        case INGREDIENT_PAGE_BAD_FOOD_ID:
            return "Provided bad ingredient id!";
        }
    }

    if (wholth_em_food_Code::_FOOD_EM_FIRST_ < ev &&
        wholth_em_food_Code::_FOOD_EM_LAST_ > ev)
    {
        switch (static_cast<wholth_em_food_Code>(ev))
        {
        case FOOD_EM_BAD_LOCALE_ID:
            return "Bad locale id provided for food management function!";
        case FOOD_EM_DUPLICATE_ENTRY:
            return "Food with same title already exists for chosen locale!";
        case _FOOD_EM_FIRST_:
        case _FOOD_EM_LAST_:
        case _FOOD_EM_COUNT_:
            return "Not an error.";
        }
    }

    if (wholth_pages_food_Code::_FOOD_PAGE_FIRST_ < ev &&
        wholth_pages_food_Code::_FOOD_PAGE_LAST_ > ev)
    {
        switch (static_cast<wholth_pages_food_Code>(ev))
        {
        case FOOD_PAGE_TYPE_MISMATCH:
            return "You are expected to provide a pointer to a page of "
                   "Foods!";
        case FOOD_PAGE_BAD_LOCALE_ID:
            return "Bad locale id provided for food page query!";
        case FOOD_PAGE_BAD_FOOD_ID:
            return "Bad food id provided for food page query!";
        case FOOD_PAGE_INGREDIENT_LIST_TOO_SHORT:
            return "Ingredient list is too short. Needs to be at least 3 "
                   "chars!";
        case _FOOD_PAGE_FIRST_:
        case _FOOD_PAGE_LAST_:
        case _FOOD_PAGE_COUNT_:
            return "Not an error.";
        case FOOD_PAGE_TITLE_TOO_SHORT:
            return "Title you are searching for is too short. Needs to be at "
                   "least 3 chars!";
            break;
        }
    }

    if (wholth_pages_nutrient_Code::_NUTRIENT_PAGE_FIRST_ < ev &&
        wholth_pages_nutrient_Code::_NUTRIENT_PAGE_LAST_ > ev)
    {
        switch (static_cast<wholth_pages_nutrient_Code>(ev))
        {
        case NUTRIENT_PAGE_TYPE_MISMATCH:
            return "You are expected to provide a pointer to a page of "
                   "Nutrients!";
        case NUTRIENT_PAGE_BAD_LOCALE_ID:
            return "Bad locale id provided for nutrient page query!";
        case _NUTRIENT_PAGE_FIRST_:
        case _NUTRIENT_PAGE_LAST_:
        case _NUTRIENT_PAGE_COUNT_:
            return "Not an error.";
        }
    }

    if (wholth_pages_food_nutrient_Code::_FOOD_NUTRIENT_PAGE_FIRST_ < ev &&
        wholth_pages_food_nutrient_Code::_FOOD_NUTRIENT_PAGE_LAST_ > ev)
    {
        switch (static_cast<wholth_pages_food_nutrient_Code>(ev))
        {
        case FOOD_NUTRIENT_PAGE_TYPE_MISMATCH:
            return "You are expected to provide a pointer to a page of "
                   "Nutrients assigned to a speific food!";
        case FOOD_NUTRIENT_PAGE_BAD_LOCALE_ID:
            return "Bad locale id provided for food nutrient page query!";
        case FOOD_NUTRIENT_PAGE_BAD_FOOD_ID:
            return "Bad food id provided for food nutrient page query!";
        case _FOOD_NUTRIENT_PAGE_FIRST_:
        case _FOOD_NUTRIENT_PAGE_LAST_:
        case _FOOD_NUTRIENT_PAGE_COUNT_:
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
