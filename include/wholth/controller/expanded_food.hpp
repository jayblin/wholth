#ifndef WHOLTH_CONTROLLER_EXPANDED_FOOD_H_
#define WHOLTH_CONTROLLER_EXPANDED_FOOD_H_

#include "wholth/model/expanded_food.hpp"
#include "wholth/task_list.hpp"

namespace wholth::controller
{
class ExpandedFood
{
  public:
    explicit ExpandedFood(
        wholth::model::ExpandedFood& m_model,
        wholth::TaskList& task_list)
        : m_model(m_model), m_task_list(task_list)
    {
    }

    auto fetch(wholth::entity::locale::id_t, sqlw::Connection& connection)
        -> void;
    auto expand(wholth::entity::locale::id_t) -> void;
    auto model() const -> const wholth::model::ExpandedFood&
    {
        return m_model;
    }
    auto clear() -> void
    {
        m_model.food = {};
    }

  private:
    wholth::model::ExpandedFood& m_model;
    wholth::TaskList& m_task_list;
};
} // namespace wholth::controller

#endif // WHOLTH_CONTROLLER_EXPANDED_FOOD_H_
