#include "wholth/controller/foods_page.hpp"
#include "wholth/list/food.hpp"

void wholth::controller::FoodsPage::update(const std::chrono::duration<double>& delta)
{
    m_model.input_timer.tick(delta);

    // @todo test
    if (m_model.input_timer.has_expired()) {
        m_task_list.add(Task::FETCH_FOODS);
    }
}

void wholth::controller::FoodsPage::on_search(uint64_t input_text_size)
{
    m_model.input_timer.start();
    m_model.title_input_size = input_text_size;
    m_model.page.current_page(0);
}

// @todo add locking
void wholth::controller::FoodsPage::fetch(
    wholth::entity::locale::id_t locale_id,
    sqlw::Connection& connection)
{
    /* m_is_fetching_foods.store(true, std::memory_order_seq_cst); */
    m_model.is_fetching.store(true, std::memory_order_seq_cst);
    auto& items = m_model.swappable_list.next().view;
    auto& buffer = m_model.swappable_list.next().buffer;
    uint64_t new_count = 0;
    wholth::list::food::list(
        items,
        new_count,
        buffer,
        {
            .page = m_model.page.current_page(),
            .locale_id = locale_id,
            .title = {m_model.title_buffer, m_model.title_input_size},
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

void wholth::controller::FoodsPage::advance()
{
    if (!m_model.is_fetching && m_model.page.advance())
    {
        m_model.input_timer.start();
    }
};

void wholth::controller::FoodsPage::retreat()
{
    if (!m_model.is_fetching && m_model.page.retreat())
    {
        m_model.input_timer.start();
    }
};
