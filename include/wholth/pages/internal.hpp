#ifndef WHLTH_PAGES_INTERNAL_H_
#define WHLTH_PAGES_INTERNAL_H_

#include "wholth/c/pages/utils.h"
#include "wholth/pages/consumption_log.hpp"
#include "wholth/pages/food.hpp"
#include "wholth/pages/food_nutrient.hpp"
#include "wholth/pages/nutrient.hpp"
#include "wholth/pages/ingredient.hpp"
#include "wholth/pages/recipe_step.hpp"
#include "wholth/pagination.hpp"
#include <cstdint>
#include <memory>
#include <variant>

namespace wholth::pages::internal
{

enum PageType : uint8_t
{
    NONE = 0,
    FOOD,
    FOOD_NUTRIENT,
    NUTRIENT,
    INGREDIENT,
    CONSUMPTION_LOG,
    RECIPE_STEP,
    _COUNT_,
};

typedef std::variant<
    std::monostate,
    wholth::pages::Food,
    wholth::pages::FoodNutrient,
    wholth::pages::Nutrient,
    wholth::pages::Ingredient,
    wholth::pages::ConsumptionLog,
    wholth::pages::RecipeStep>
    PageData;

static_assert(std::variant_size_v<PageData> == PageType::_COUNT_);

} // namespace wholth::pages::internal

struct wholth_Page_t
{
    enum class State : int
    {
        FREE = 0,
        FETCHING,
        OCCUPIED,
    };

    wholth::Pagination pagination{0};
    std::atomic<State> state{State::FREE};
    wholth::pages::internal::PageData data{std::monostate{}};

    wholth_Page_t();
    wholth_Page_t(
        wholth::Pagination::per_page_t _per_page,
        wholth::pages::internal::PageData _data);

    wholth_Page_t(const wholth_Page_t&) = delete;
    wholth_Page_t& operator=(const wholth_Page_t&) = delete;

    wholth_Page_t(wholth_Page_t&& other)
    {
        *this = std::move(other);
    }
    wholth_Page_t& operator=(wholth_Page_t&&);
};

#endif // WHLTH_PAGES_INTERNAL_H_
