#include "wholth/context.hpp"
#include "wholth/entity/food.hpp"

void wholth::Context::locale_id(std::string new_locale_id)
{
    // @todo add lock?
    m_locale_id = std::move(new_locale_id);

    m_task_queue.enqueue(Task::FETCH_FOODS);
}

void wholth::Context::update(const std::chrono::duration<double>& delta)
{
    foods_page_ctrl.update(delta);

    if (m_task_queue.is_empty())
    {
        return;
    }

    std::thread th1(
        [this](typeof(m_task_queue) tasks) {
            std::lock_guard lock{m_task_mutex};

            for (TaskQueue::event_t t = 0; t < tasks.queue().size(); t++)
            {
                if (tasks.queue()[t] == 0)
                {
                    continue;
                }

                switch (static_cast<Task>(t))
                {
                case Task::FETCH_FOODS:
                    foods_page_ctrl.fetch(m_locale_id, connection);
                    foods_page_ctrl.clear_expanded_food();
                    break;
                case FETCH_EXPANDED_FOOD_INFO:
                    foods_page_ctrl.fetch_expanded_food(
                        m_locale_id, connection);
                    break;
                    /* } */
                    /* case Task::_LAST_: */
                    /*     break; */
                default:
                    break;
                }
            }

            /* auto& ingredient_page = m_ingredient_pages.next(); */
            /* wholth::list::food::list( */
            /*     ingredient_page.m_items, */
            /*     ingredient_page.m_count, */
            /*     ingredient_page.m_buffer, */
            /*     { */
            /*         .page = ingredient_page.m_cur_page, */
            /*         .locale_id = m_locale_id, */
            /*         .title = m_ingredient_title_buffer, */
            /*     }, */
            /*     connection */
            /* ); */
            /* m_ingredient_pages.swap(); */
        },
        m_task_queue);

    th1.detach();

    for (TaskQueue::event_t i = 0; i < m_task_queue.queue().size(); i++)
    {
        m_task_queue.dequeue(static_cast<Task>(i));
    }

    /* if (!m_is_fetching_list && m_timer.has_expired()) */
    /* { */
    /*     fetch(m_query); */
    /* } */
}
