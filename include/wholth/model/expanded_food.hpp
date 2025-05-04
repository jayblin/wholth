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
};

// todo move to another file or rename
template <wholth::concepts::is_nutrient T, size_t Size = 20>
using NutrientsContainer = SwappableBufferViewsAwareContainer<T, Size>;

} // namespace wholth::model

#endif // WHOLTH_MODEL_EXPANDED_FOOD_H_
