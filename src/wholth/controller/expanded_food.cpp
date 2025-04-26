#include "wholth/model/expanded_food.hpp"
#include "wholth/controller/expanded_food.hpp"
#include <iostream>

/* template <typename T> */
/* template <typename Food, typename Nutrient, typename RecipeStep> */
/* void wholth::controller::ExpandedFood::fetch( */
/*     model_t& m_model, */
/*     wholth::entity::locale::id_t locale_id, */
/*     sqlw::Connection& connection) */
/* { */
/*     // todo: check error codes. */
/*     /1* wholth::expand_food( *1/ */
/*     /1*     m_model.food, m_model.food_buffer, m_model.food.id, locale_id, &connection); *1/ */
/*     /1* wholth::list_nutrients( *1/ */
/*     /1*     m_model.nutrients, m_model.nutrients_buffer, m_model.food.id, locale_id, &connection); *1/ */
/*     /1* wholth::list_steps( *1/ */
/*     /1*     m_model.steps, m_model.steps_buffer, m_model.food.id, locale_id, &connection); *1/ */

/*     /1* std::cout *1/ */
/*     /1*     << "food_buf " << m_model.food_buffer << '\n' *1/ */
/*     /1*     << "nut_buf " << m_model.nutrients_buffer << '\n' *1/ */
/*     /1*     << "step_buf " << m_model.steps_buffer << '\n' *1/ */
/*     /1* ; *1/ */
/* } */

/* template <typename T> */
/* void wholth::controller::ExpandedFood::expand(model_t& m_model, wholth::TaskList& m_task_list, wholth::entity::food::id_t food_id) */
/* { */
/*     if (m_model.food.id != food_id) { */
/*         m_task_list.add(wholth::Task::FETCH_EXPANDED_FOOD_INFO); */
/*         m_model.food.id = food_id; */
/*         m_model.should_show = true; */
/*     } */
/*     else { */
/*         m_model.should_show = !m_model.should_show; */
/*     } */
/* } */
