#include "wholth/c/food_ingrediets.h"
#include "wholth/c/internal.hpp"
#include "wholth/controller/abstract_page.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/model/expanded_food.hpp"

static_assert(sizeof(wholth::entity::Ingredient) == sizeof(wholth_Ingredient));

static auto& g_context = wholth::c::internal::global_context();

static wholth::model::FoodIngredientsContainer<wholth_Ingredient>
    g_ingredients_container;

static auto& model()
{
    static wholth::model::FoodIngredients g_ingredients_model{
        g_context, g_ingredients_container.size};
    return g_ingredients_model;
}

extern "C" const wholth_IngredientArray wholth_food_ingredients()
{
    const auto& vector =
        g_ingredients_container.swappable_buffer_views.view_current().view;
    const auto& pagination = model().pagination;

    assertm(
        vector.size() >= pagination.span_size(),
        "You done goofed here wholth_food_ingredients() [1]!");
    return {vector.data(), pagination.span_size()};
}

extern "C" wholth_ErrorCode wholth_food_ingredients_food_id(
    const wholth_StringView food_id)
{
    model().food_id = {food_id.data, food_id.size};
    return WHOLTH_NO_ERROR;
}

extern "C" wholth_ErrorCode wholth_food_ingredients_fetch()
{
    wholth::controller::fill_container_through_model(
        g_ingredients_container, model(), g_context.connection);

    return WHOLTH_NO_ERROR;
}
