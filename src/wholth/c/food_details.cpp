#include "wholth/c/food_details.h"
#include "wholth/c/internal.hpp"
#include "wholth/controller/abstract_page.hpp"
#include "wholth/model/expanded_food.hpp"

static_assert(
    sizeof(wholth::entity::food::Details) == sizeof(wholth_FoodDetails));
static auto& g_context = wholth::c::internal::global_context();

static wholth::model::FoodDetailsContainer<wholth_FoodDetails>
    g_food_details_container;

static auto& model()
{
    static wholth::model::FoodDetails g_food_details_model{g_context};
    return g_food_details_model;
}

extern "C" const wholth_FoodDetails wholth_food_details()
{
    return g_food_details_container.current().view;
}

extern "C" void wholth_food_details_food_id(const wholth_StringView food_id)
{
    model().food_id = {food_id.data, food_id.size};
}

extern "C" wholth_Error wholth_food_details_fetch()
{
    const auto ec = wholth::controller::fill_object_through_model(
        g_food_details_container, model(), g_context.connection);

    return wholth::c::internal::check_push_and_get(ec);
}
