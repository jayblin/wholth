#include "wholth/c/ingredients.h"
#include "wholth/c/internal.hpp"
#include "wholth/controller/abstract_page.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/model/ingredients.hpp"
#include "wholth/utils/to_string_view.hpp"

// todo rename this file because not all function start with "ingredients"?

static_assert(sizeof(wholth::entity::Ingredient) == sizeof(wholth_Ingredient));

static auto& g_context = wholth::c::internal::global_context();

static wholth::model::FoodIngredientsContainer<wholth_Ingredient, 5>
    g_foods_container;

static auto& model()
{
    static wholth::model::Ingredients g_ingredients_model{
        g_context, g_foods_container.size};
    return g_ingredients_model;
}

extern "C" bool wholth_ingredients_advance(uint64_t by)
{
    return wholth::controller::advance(model(), by);
}

extern "C" bool wholth_ingredients_retreat(uint64_t by)
{
    return wholth::controller::retreat(model(), by);
}

extern "C" bool wholth_ingredients_skip_to(uint64_t page)
{
    return wholth::controller::skip_to(model(), page);
}

extern "C" void wholth_ingredients_fetch()
{
    // todo fix warn
    const auto ec = wholth::controller::fill_container_through_model(
        g_foods_container, model(), g_context.connection);
}

extern "C" void wholth_ingredients_title(wholth_StringView search_title)
{
    model().title = {search_title.data, search_title.size};
}

extern "C" const wholth_IngredientArray wholth_ingredients()
{
    /* if (sqlw::status::Condition::OK != g_context.connection.status()) */
    /* { */
    /*     // todo show error here? */
    /*     return {nullptr, 0}; */
    /* } */

    const auto& vector =
        g_foods_container.swappable_buffer_views.view_current().view;

    const auto& pagination = model().pagination;

    assertm(
        vector.size() >= pagination.span_size(),
        "You done goofed here wholth_ingredients() [1]!");
    return {vector.data(), pagination.span_size()};
}

extern "C" bool wholth_ingredients_is_fetching()
{
    return model().is_fetching;
}

extern "C" wholth_Page wholth_ingredients_pagination()
{
    const auto& page = model().pagination;

    return {
        .max_page = page.max_page(), .cur_page = page.current_page(),
        /* .pagination = { */
        /*     .data = page.pagination().data(), */
        /*     .size = page.pagination().size(), */
        /* }}; */
    };
}
