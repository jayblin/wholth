#include "wholth/model/expanded_food.hpp"
#include "wholth/controller/expanded_food.hpp"
#include <iostream>

void wholth::controller::ExpandedFood::fetch(
    wholth::entity::locale::id_t locale_id,
    sqlw::Connection& connection)
{
    // todo: check error codes.
    /* wholth::expand_food( */
    /*     m_model.food, m_model.food_buffer, m_model.food.id, locale_id, &connection); */
    /* wholth::list_nutrients( */
    /*     m_model.nutrients, m_model.nutrients_buffer, m_model.food.id, locale_id, &connection); */
    /* wholth::list_steps( */
    /*     m_model.steps, m_model.steps_buffer, m_model.food.id, locale_id, &connection); */

    /* std::cout */
    /*     << "food_buf " << m_model.food_buffer << '\n' */
    /*     << "nut_buf " << m_model.nutrients_buffer << '\n' */
    /*     << "step_buf " << m_model.steps_buffer << '\n' */
    /* ; */
}

void wholth::controller::ExpandedFood::expand(wholth::entity::food::id_t food_id)
{
    if (m_model.food.id != food_id) {
        m_task_queue.enqueue(wholth::Task::FETCH_EXPANDED_FOOD_INFO);
        m_model.food.id = food_id;
        m_model.should_show = true;
    }
    else {
        m_model.should_show = !m_model.should_show;
    }
}
