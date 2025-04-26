#ifndef WHOLTH_CONTROLLER_FOODS_PAGE_H_
#define WHOLTH_CONTROLLER_FOODS_PAGE_H_

#include "sqlw/forward.hpp"
#include "wholth/controller/expanded_food.hpp"
#include "wholth/list.hpp"
#include "wholth/list/food.hpp"
#include "wholth/model/foods_page.hpp"
#include "wholth/scheduler.hpp"
#include "wholth/task_list.hpp"

namespace wholth::controller
{
template <typename T = wholth::entity::shortened::Food>
/* template <typename T> */
class FoodsPage
{
  public:
    typedef wholth::model::FoodsPage<T> model_t;

    /* FoodsPage(wholth::model::FoodsPage& m_model, wholth::TaskList& task_list)
     */
    FoodsPage(model_t& model) : m_model(model)
    /*     : */
    /*         /1* m_model(m_model), *1/ */
    /*       /1* m_expanded_food_ctrl(m_model.expanded_food, task_list), *1/ */
    /*       m_task_list(task_list) */
    {
    }

    auto fetch(
        wholth::entity::locale::id_t locale_id,
        sqlw::Connection& connection) -> void;
    auto advance() -> bool;
    auto retreat() -> bool;
    auto on_search(uint64_t input_text_size) -> void;
    /* auto expand_food(wholth::entity::locale::id_t food_id) -> void */
    /* { */
    /*     m_expanded_food_ctrl.expand(food_id); */
    /* } */
    /* auto clear_expanded_food() -> void */
    /* { */
    /*     m_expanded_food_ctrl.clear(); */
    /* } */
    /* auto fetch_expanded_food( */
    /*     wholth::entity::locale::id_t locale_id, */
    /*     sqlw::Connection& connection) -> void */
    /* { */
    /*     m_expanded_food_ctrl.fetch(locale_id, connection); */
    /* } */
    // todo remake to model_t
    auto model() const -> const wholth::model::FoodsPage<>&
    {
        return m_model;
    }
    /* auto title_buffer() -> char* */
    /* { */
    /*     return m_model.title_buffer; */
    /* } */
    /* auto sizeof_title_buffer() const -> size_t */
    /* { */
    /*     return sizeof(m_model.title_buffer); */
    /* } */
    /* auto update( */
    /*     const std::chrono::duration<double>& delta */
    /*     /1* wholth::TaskList& m_task_list *1/ */
    /*     ) -> void; */

  private:
    model_t& m_model;
    /* wholth::controller::ExpandedFood m_expanded_food_ctrl; */
    /* wholth::TaskList& m_task_list; */
};
} // namespace wholth::controller

template <typename T>
void wholth::controller::FoodsPage<T>::on_search(uint64_t input_text_size)
{
    /* m_model.input_timer.start(); */
    m_model.title_input_size = input_text_size;
    m_model.page.current_page(0);
}

// @todo add locking
template <typename T>
void wholth::controller::FoodsPage<T>::fetch(
    wholth::entity::locale::id_t locale_id,
    sqlw::Connection& connection)
{
    if (sqlw::status::Condition::OK != connection.status()) {
        std::cout << "NOT CONNECTED TO DB\n";
        // todo panic here maybe?
        return;
    }

    /* m_is_fetching_foods.store(true, std::memory_order_seq_cst); */
    m_model.is_fetching.store(true, std::memory_order_seq_cst);
    auto& items = m_model.swappable_list.next().view;
    auto& buffer = m_model.swappable_list.next().buffer;
    uint64_t new_count = 0;
    /* wholth::list::food::list< */
    /*     std::remove_reference_t<decltype(items)>::value_type>( */
    /* const auto ec = wholth::list::food::list<T>( */
    const auto ec = wholth::fill_span<T>(
        items,
        /* new_count, */
        buffer,
        new_count,
        wholth::list::food::Query{
            .page = m_model.page.current_page(),
            .locale_id = locale_id,
            /* .title = {m_model.title_buffer, m_model.title_input_size}, */
        },
        connection);

    m_model.page.count(new_count);

    m_model.page.update();
    m_model.swappable_list.swap();

    for (auto& food : m_model.swappable_list.next().view)
    {
        food = {};
    }

    // @todo learn about memeory order
    m_model.is_fetching.store(false, std::memory_order_seq_cst);
}

template <typename T>
bool wholth::controller::FoodsPage<T>::advance()
{
    return !m_model.is_fetching && m_model.page.advance();
    /* if (!m_model.is_fetching && m_model.page.advance()) */
    /* { */
    /*     /1* m_model.input_timer.start(); *1/ */
    /* } */
};

template <typename T>
bool wholth::controller::FoodsPage<T>::retreat()
{
    return !m_model.is_fetching && m_model.page.retreat();
    /* if (!m_model.is_fetching && m_model.page.retreat()) */
    /* { */
    /*     /1* m_model.input_timer.start(); *1/ */
    /* } */
};

#endif // WHOLTH_CONTROLLER_FOODS_PAGE_H_
