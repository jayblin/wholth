#include "wholth/context.hpp"
#include <tuple>

/* void wholth::Context::locale_id(std::string new_locale_id) */
/* { */
/*     // @todo add lock? */
/*     m_locale_id = std::move(new_locale_id); */

/*     /1* m_task_list.add(Task::FETCH_FOODS); *1/ */
/*     /1* scheduler.schedule(Task::FETCH_FOODS); *1/ */
/* } */

/* template <typename... M, typename... C> */
template <typename... C>
void wholth::Context::update(
    const std::chrono::duration<double>& delta,
    /* std::tuple<M...>& models, */
    std::tuple<C...>& controllers
)
{
    /* /1* ctrl.update( *1/ */
    /* /1*     delta, *1/ */
    /* /1*     std::get<wholth::model::FoodsPage>(models), *1/ */
    /* /1*     m_task_list *1/ */
    /* /1* ); *1/ */
    /* /1* foods_page_ctrl.update(delta); *1/ */

    /* /1* if (m_task_list.is_empty()) *1/ */
    /* /1* { *1/ */
    /* /1*     return; *1/ */
    /* /1* } *1/ */
    /* scheduler.tick(delta); */

    /* wholth::controller::FoodsPage<>& ctrl = std::get(controllers); */
    /* auto m_task_list = scheduler.get_due(); */

    /* std::thread th1( */
    /*     // EXPLICIT COPY OF m_task_list is IMPORTANT!!! */
    /*     [this, &ctrl](typeof(m_task_list) task_list) { */
    /*         std::lock_guard lock{m_task_mutex}; */

    /*         if (task_list.has(Task::FETCH_FOODS)) { */
    /*                 ctrl.fetch( */
    /*                     m_locale_id, */
    /*                     connection */
    /*                 ); */
    /*                 /1* ctrl.fetch(m_locale_id, connection); *1/ */
    /*                 /1* ctrl.clear_expanded_food(); *1/ */
    /*         } */
    /*         if (task_list.has(Task::FETCH_EXPANDED_FOOD_INFO)) { */
    /*                 /1* ctrl.fetch_expanded_food( *1/ */
    /*                 /1*     m_locale_id, connection); *1/ */
    /*         } */

    /*         /1* for (TaskQueue::event_t t = 0; t < tasks.queue().size(); t++) *1/ */
    /*         /1* { *1/ */
    /*         /1*     if (tasks.queue()[t] == 0) *1/ */
    /*         /1*     { *1/ */
    /*         /1*         continue; *1/ */
    /*         /1*     } *1/ */

    /*         /1*     switch (static_cast<Task>(t)) *1/ */
    /*         /1*     { *1/ */
    /*         /1*     case Task::FETCH_FOODS: *1/ */
    /*         /1*         foods_page_ctrl.fetch(m_locale_id, connection); *1/ */
    /*         /1*         foods_page_ctrl.clear_expanded_food(); *1/ */
    /*         /1*         break; *1/ */
    /*         /1*     case FETCH_EXPANDED_FOOD_INFO: *1/ */
    /*         /1*         foods_page_ctrl.fetch_expanded_food( *1/ */
    /*         /1*             m_locale_id, connection); *1/ */
    /*         /1*         break; *1/ */
    /*         /1*         /2* } *2/ *1/ */
    /*         /1*         /2* case Task::_LAST_: *2/ *1/ */
    /*         /1*         /2*     break; *2/ *1/ */
    /*         /1*     default: *1/ */
    /*         /1*         break; *1/ */
    /*         /1*     } *1/ */
    /*         /1* } *1/ */

    /*         /1* auto& ingredient_page = m_ingredient_pages.next(); *1/ */
    /*         /1* wholth::list::food::list( *1/ */
    /*         /1*     ingredient_page.m_items, *1/ */
    /*         /1*     ingredient_page.m_count, *1/ */
    /*         /1*     ingredient_page.m_buffer, *1/ */
    /*         /1*     { *1/ */
    /*         /1*         .page = ingredient_page.m_cur_page, *1/ */
    /*         /1*         .locale_id = m_locale_id, *1/ */
    /*         /1*         .title = m_ingredient_title_buffer, *1/ */
    /*         /1*     }, *1/ */
    /*         /1*     connection *1/ */
    /*         /1* ); *1/ */
    /*         /1* m_ingredient_pages.swap(); *1/ */
    /*     }, */
    /*     m_task_list); */

    /* th1.detach(); */

    /* m_task_list.clear(); */
    /* /1* for (TaskQueue::event_t i = 0; i < m_task_queue.queue().size(); i++) *1/ */
    /* /1* { *1/ */
    /* /1*     m_task_queue.dequeue(static_cast<Task>(i)); *1/ */
    /* /1* } *1/ */

    /* /1* if (!m_is_fetching_list && m_timer.has_expired()) *1/ */
    /* /1* { *1/ */
    /* /1*     fetch(m_query); *1/ */
    /* /1* } *1/ */
}
