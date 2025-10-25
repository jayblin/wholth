#ifndef WHOLTH_MODEL_EXPANDED_FOOD_H_
#define WHOLTH_MODEL_EXPANDED_FOOD_H_

#include "wholth/context.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/hydrate_describable.hpp"
#include "wholth/hydrate_ingredient.hpp"
#include "wholth/hydrate_nutrient.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/page.hpp"

namespace wholth::model
{

struct FoodDetails
{
    const wholth::Context& ctx;
    std::atomic<bool> is_fetching{false};
    /* wholth::entity::food::id_t food_id{""}; */
    std::string food_id{""};
};

template <wholth::concepts::is_describable T>
using FoodDetailsContainer = wholth::Swappable<wholth::BufferView<T>>;

struct FoodIngredients
{
    const wholth::Context& ctx;
    wholth::Pagination pagination;
    std::atomic<bool> is_fetching{false};
    /* wholth::entity::food::id_t food_id{""}; */
    std::string food_id{""};
};

template <wholth::concepts::is_ingredient T, size_t Size = 20>
using FoodIngredientsContainer = SwappableBufferViewsAwareContainer<T, Size>;

} // namespace wholth::model

#endif // WHOLTH_MODEL_EXPANDED_FOOD_H_
