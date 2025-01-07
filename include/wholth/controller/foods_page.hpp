#ifndef WHOLTH_CONTROLLER_FOODS_PAGE_H_
#define WHOLTH_CONTROLLER_FOODS_PAGE_H_

#include "wholth/controller/expanded_food.hpp"
#include "wholth/model/foods_page.hpp"
#include "wholth/task_list.hpp"

namespace wholth::controller
{
class FoodsPage
{
  public:
    FoodsPage(wholth::model::FoodsPage& m_model, wholth::TaskList& task_list)
        : m_model(m_model),
          m_expanded_food_ctrl(m_model.expanded_food, task_list),
          m_task_list(task_list)
    {
    }

    auto fetch(
        wholth::entity::locale::id_t locale_id,
        sqlw::Connection& connection) -> void;
    auto advance() -> void;
    auto retreat() -> void;
    auto on_search(uint64_t input_text_size) -> void;
    auto expand_food(wholth::entity::locale::id_t food_id) -> void
    {
        m_expanded_food_ctrl.expand(food_id);
    }
    auto clear_expanded_food() -> void
    {
        m_expanded_food_ctrl.clear();
    }
    auto fetch_expanded_food(
        wholth::entity::locale::id_t locale_id,
        sqlw::Connection& connection) -> void
    {
        m_expanded_food_ctrl.fetch(locale_id, connection);
    }
    auto model() const -> const wholth::model::FoodsPage&
    {
        return m_model;
    }
    auto title_buffer() -> char*
    {
        return m_model.title_buffer;
    }
    auto sizeof_title_buffer() const -> size_t
    {
        return sizeof(m_model.title_buffer);
    }
    auto update(const std::chrono::duration<double>& delta) -> void;

  private:
    wholth::model::FoodsPage& m_model;
    wholth::controller::ExpandedFood m_expanded_food_ctrl;
    wholth::TaskList& m_task_list;
};
} // namespace wholth::controller

#endif // WHOLTH_CONTROLLER_FOODS_PAGE_H_
