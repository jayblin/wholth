#ifndef WHOLTH_MODEL_EXPANDED_FOOD_H_
#define WHOLTH_MODEL_EXPANDED_FOOD_H_

#include "wholth/buffer_view.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/hydrate.hpp"
#include "wholth/page.hpp"
#include <span>
#include <vector>

// todo: move to model namespace
namespace wholth::model
{
template<wholth::concepts::is_describable D, wholth::concepts::is_nutrient Nutrient, typename RecipeStep, typename Ingredient>
struct ExpandedFood
{
    /* bool should_show{false}; */
    BufferView<D> details;

    BufferView<std::array<RecipeStep, 1>> steps {};

    BufferView<std::array<Ingredient, 30>> ingredients {};

    /* BufferView<std::array<Nutrient, 500>> nutrients; */
    BufferView<std::array<Nutrient, 500>> nutrients {};
};

template<wholth::concepts::is_nutrient Nutrient>
struct NutrientsPage
{
    NutrientsPage()
    {
        nutrients.view.resize(20);
    }

    Page page {20};
    BufferView<std::vector<Nutrient>> nutrients {};
};


} // namespace wholth::model

#endif // WHOLTH_MODEL_EXPANDED_FOOD_H_
