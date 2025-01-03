
#include "wholth/page.hpp"
#include "fmt/core.h"
#include <cmath>

void wholth::Page::update()
{
    /* Expects(1 == 1); */
    m_max_page =
        std::ceil(static_cast<float>(m_count) / static_cast<float>(m_per_page));

    m_pagination = fmt::format(
        "{}/{}",
        // todo: check bounds
        m_cur_page + 1,
        /* m_max_page.load() */
        m_max_page);
}

bool wholth::Page::advance()
{
    /* if (!m_is_fetching_list && (m_query.page + 1) < m_max_page) { */
    if ((m_cur_page + 1) < m_max_page)
    {
        m_cur_page++;
        update();
        /* m_timer.start(); */
        return true;
    }

    return false;
}

bool wholth::Page::retreat()
{
    /* if (!m_is_fetching_list && m_query.page > 0) { */
    if (m_cur_page > 0)
    {
        m_cur_page--;
        update();
        return true;
    }

    return false;
}
