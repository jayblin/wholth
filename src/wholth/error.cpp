#include "wholth/error.hpp"
#include "wholth/c/pages/food_nutrient.h"
#include "wholth/c/pages/nutrient.h"
#include <string_view>

const char* wholth::error::Category::name() const noexcept
{
    return "wholth";
}

std::string_view wholth::error::message(int ev)
{
    static_assert(
        (int)wholth_pages_nutrient_Code::_NUTRIENT_PAGE_FIRST_ !=
        (int)wholth_pages_food_nutrient_Code::_FOOD_NUTRIENT_PAGE_FIRST_);

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
