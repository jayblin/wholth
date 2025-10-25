#include "wholth/c/entity/recipe_step.h"
#include "wholth/entity/recipe_step.hpp"

static_assert(sizeof(wholth::entity::RecipeStep) == sizeof(wholth_RecipeStep));

// static auto& g_context = wholth::c::internal::global_context();
//
// static wholth::model::RecipeStepContainer<wholth_RecipeStep>
//     g_recipe_step_container;
//
// static auto& model()
// {
//     static wholth::model::RecipeStep g_recipe_step_model{g_context};
//     return g_recipe_step_model;
// }
//
// // not thread safe
// extern "C" wholth_Error wholth_recipe_step_insert(
//     wholth_RecipeStep* step,
//     const wholth_Food* food)
// {
//     const wholth::entity::RecipeStep _step{
//         .description = wholth::utils::to_string_view(step->description),
//     };
//     const wholth::entity::Food& _food{
//         .id = wholth::utils::to_string_view(food->id),
//     };
//     // yucky
//     static std::string result_buffer;
//     const auto ec = wholth::controller::insert(
//         _step, result_buffer, g_context.connection, g_context.locale_id, _food);
//
//     step->id = wholth_StringView{
//         .data = result_buffer.data(), .size = result_buffer.size()};
//
//     return wholth::c::internal::check_push_and_get(ec);
// }
//
// extern "C" const wholth_RecipeStep wholth_recipe_step()
// {
//     return g_recipe_step_container.view_current().view;
// }
//
// extern "C" void wholth_recipe_step_food_id(const wholth_StringView food_id)
// {
//     model().food_id = {food_id.data, food_id.size};
// }
//
// extern "C" wholth_Error wholth_recipe_step_fetch()
// {
//     const auto ec = wholth::controller::fill_object_through_model(
//         g_recipe_step_container, model(), g_context.connection);
//
//     return wholth::c::internal::check_push_and_get(ec);
// }
