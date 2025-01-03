#ifndef WHOLTH_CONTROLLER_EXPANDED_FOOD_H_
#define WHOLTH_CONTROLLER_EXPANDED_FOOD_H_

#include "wholth/model/expanded_food.hpp"
#include "wholth/task_queue.hpp"

namespace wholth::controller
{
class ExpandedFood
{
  public:
    explicit ExpandedFood(
        wholth::model::ExpandedFood& m_model,
        wholth::TaskQueue& task_queue)
        : m_model(m_model), m_task_queue(task_queue)
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
    wholth::TaskQueue& m_task_queue;
};
} // namespace wholth::controller

#endif // WHOLTH_CONTROLLER_EXPANDED_FOOD_H_
