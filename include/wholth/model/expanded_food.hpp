#ifndef WHOLTH_MODEL_EXPANDED_FOOD_H_
#define WHOLTH_MODEL_EXPANDED_FOOD_H_

#include "wholth/context.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/hydrate.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/page.hpp"

namespace wholth::model
{

// todo move to another file or rename
struct NutrientsPage
{
    const wholth::Context& ctx;
    wholth::Pagination pagination;
    std::atomic<bool> is_fetching{false};
    wholth::entity::food::id_t food_id{""};
    wholth::entity::nutrient::title_t title{""};
};

// todo move to another file or rename
template <wholth::concepts::is_nutrient T, size_t Size = 20>
using NutrientsContainer = SwappableBufferViewsAwareContainer<T, Size>;

struct FoodDetails
{
    const wholth::Context& ctx;
    std::atomic<bool> is_fetching{false};
    wholth::entity::food::id_t food_id{""};
};

template <wholth::concepts::is_describable T>
using FoodDetailsContainer = wholth::Swappable<wholth::BufferView<T>>;

} // namespace wholth::model

#endif // WHOLTH_MODEL_EXPANDED_FOOD_H_
