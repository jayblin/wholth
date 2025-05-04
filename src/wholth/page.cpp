
#include "wholth/page.hpp"
#include "fmt/core.h"
#include <cmath>

void wholth::Pagination::update()
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

bool wholth::Pagination::advance(uint64_t pages)
{
    auto new_cur_page = m_cur_page + pages;

    if (0 != m_count &&
        (new_cur_page > m_max_page || new_cur_page < m_cur_page))
    {
        new_cur_page = m_max_page;
    }

    if (m_cur_page != new_cur_page)
    {
        m_cur_page = new_cur_page;
        update();
        return true;
    }

    return false;
}

bool wholth::Pagination::retreat(uint64_t pages)
{
    auto new_cur_page = m_cur_page - pages;

    if (0 != m_count &&
        (new_cur_page > m_max_page || new_cur_page > m_cur_page))
    {
        new_cur_page = 0;
    }

    if (m_cur_page != new_cur_page)
    {
        m_cur_page = new_cur_page;
        update();
        return true;
    }

    return false;
}
