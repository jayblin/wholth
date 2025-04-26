#ifndef WHOLTH_CONTROLLER_EXPANDED_FOOD_H_
#define WHOLTH_CONTROLLER_EXPANDED_FOOD_H_

#include "wholth/app_c.h"
#include "wholth/buffer_view.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list.hpp"
#include "wholth/list/nutrient.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/status.hpp"
#include "wholth/task_list.hpp"
#include <tuple>
#include "utils/timer.hpp"
#include "wholth/utils.hpp"

namespace wholth::controller
{
template <
    wholth::concepts::is_describable D,
    wholth::concepts::is_nutrient N,
    typename RecipeStep,
    typename Ingredient>
class ExpandedFood
{
  public:
    typedef wholth::model::ExpandedFood<D, N, RecipeStep, Ingredient> model_t;
    explicit ExpandedFood(model_t& model) : m_model(model)
    {
    }

    auto fetch_details(
        wholth::entity::food::id_t,
        wholth::entity::locale::id_t,
        sqlw::Connection& connection) -> std::error_code;
    auto fetch_recipe_steps(
        wholth::entity::food::id_t,
        wholth::entity::locale::id_t,
        sqlw::Connection& connection) -> std::error_code;
    auto fetch_ingredients(
        wholth::entity::food::id_t,
        wholth::entity::locale::id_t,
        sqlw::Connection& connection) -> std::error_code;
    auto model() const -> const model_t&
    {
        return m_model;
    }
    /* auto expand(model_t&, wholth::TaskList&, wholth::entity::locale::id_t) */
    /* -> void; */
    /* auto model() const -> const wholth::model::ExpandedFood& */
    /* { */
    /*     return m_model; */
    /* } */
    /* auto clear(model_t& m_model) -> void */
    /* { */
    /*     m_model.food = {}; */
    /* } */

  private:
    model_t& m_model;
    /* wholth::model::ExpandedFood& m_model; */
    /* wholth::TaskList& m_task_list; */
};

template <wholth::concepts::is_nutrient N>
class NutrientsPage
{
  public:
    typedef wholth::model::NutrientsPage<N> model_t;
    explicit NutrientsPage(model_t& model) : m_model(model)
    {
    }

    auto fetch_nutrients(
        wholth::entity::food::id_t,
        wholth::entity::locale::id_t,
        sqlw::Connection& connection) -> std::error_code;

  private:
    model_t& m_model;
};

} // namespace wholth::controller

template <
    wholth::concepts::is_describable D,
    wholth::concepts::is_nutrient N,
    typename R,
    typename I>
std::error_code wholth::controller::ExpandedFood<D, N, R, I>::fetch_details(
    wholth::entity::food::id_t food_id,
    wholth::entity::locale::id_t locale_id,
    sqlw::Connection& connection)
{
    wholth::BufferView<wholth::utils::LengthContainer> lc;
    const auto ec =
        wholth::entity::food::fill_details(food_id, locale_id, lc, connection);

    /* assert(lc.view.next) */

    if (wholth::status::Condition::OK != ec)
    {
        return ec;
    }

    m_model.details.view.description =
        lc.view.next<decltype(m_model.details.view.description)>(lc.buffer);
    m_model.details.buffer = std::move(lc.buffer);

    return ec;
    /* if (wholth::status::Condition::OK != ec) { */
    /* } */

    // todo: check error codes.
    /* wholth::expand_food( */
    /*     m_model.food, m_model.food_buffer, m_model.food.id, locale_id,
     * &connection); */
    /* wholth::list_nutrients( */
    /*     m_model.nutrients, m_model.nutrients_buffer, m_model.food.id,
     * locale_id, &connection); */
    /* wholth::list_steps( */
    /*     m_model.steps, m_model.steps_buffer, m_model.food.id, locale_id,
     * &connection); */

    /* std::cout */
    /*     << "food_buf " << m_model.food_buffer << '\n' */
    /*     << "nut_buf " << m_model.nutrients_buffer << '\n' */
    /*     << "step_buf " << m_model.steps_buffer << '\n' */
    /* ; */
}

template <wholth::concepts::is_nutrient N>
std::error_code wholth::controller::NutrientsPage<N>::fetch_nutrients(
    wholth::entity::food::id_t food_id,
    wholth::entity::locale::id_t locale_id,
    sqlw::Connection& db_con)
{
    wholth::list::nutrient::FoodDependentQuery query{
        .page = m_model.page.current_page(),
        .locale_id = locale_id,
        .food_id = food_id};

    uint64_t new_count = 0;
    const auto ec = wholth::fill_span<N>(
        m_model.nutrients.view,
        m_model.nutrients.buffer,
        new_count,
        query,
        db_con);
    m_model.page.count(new_count);

    // todo check ec

    return ec;
}

template <
    wholth::concepts::is_describable D,
    wholth::concepts::is_nutrient N,
    typename R,
    typename I>
std::error_code wholth::controller::ExpandedFood<D, N, R, I>::
    fetch_recipe_steps(
        wholth::entity::food::id_t food_id,
        wholth::entity::locale::id_t locale_id,
        sqlw::Connection& connection)
{
    const auto ec = wholth::entity::food::fill_recipe_steps(
        food_id,
        m_model.steps.view,
        locale_id,
        m_model.steps.buffer,
        connection);

    return ec;
}

template <
    wholth::concepts::is_describable D,
    wholth::concepts::is_nutrient N,
    typename R,
    typename I>
std::error_code wholth::controller::ExpandedFood<D, N, R, I>::fetch_ingredients(
    wholth::entity::food::id_t food_id,
    wholth::entity::locale::id_t locale_id,
    sqlw::Connection& connection)
{
    const auto ec = wholth::entity::food::fill_ingredients(
        food_id,
        m_model.ingredients.view,
        locale_id,
        m_model.ingredients.buffer,
        connection);

    return ec;
}

#endif // WHOLTH_CONTROLLER_EXPANDED_FOOD_H_
