#ifndef WHOLTH_PAGE_H_
#define WHOLTH_PAGE_H_

#include <cstdint>
#include <string>
#include <string_view>

namespace wholth
{
class Page
{
  public:
    Page(uint64_t per_page) : m_per_page(per_page)
    {
    }

    // todo rename to `update_pagination()`?
    auto update() -> void;
    auto advance() -> bool;
    auto retreat() -> bool;

    auto pagination() const -> std::string_view
    {
        return m_pagination;
    };
    auto count() const -> uint64_t
    {
        return m_count;
    };
    auto count(uint64_t new_count) -> void
    {
        m_count = new_count;
    };
    /*
     * Returns current page zero indexed.
     */
    auto current_page() const -> uint64_t
    {
        return m_cur_page;
    };
    auto current_page(uint64_t new_cur_page) -> void
    {
        m_cur_page = new_cur_page;
    };
    auto max_page() const -> uint64_t
    {
        return m_max_page - 1;
    }
    /* auto is_last_page() const -> bool */
    /* { */
    /*     return (m_cur_page + 1) == m_max_page; */
    /* } */
    /* auto is_first_page() const -> bool */
    /* { */
    /*     return 0 == m_cur_page; */
    /* } */

  protected:
    uint64_t m_per_page{0};
    uint64_t m_count{0};
    uint64_t m_cur_page{0};
    uint64_t m_max_page{1};
    std::string m_pagination{""};
};
} // namespace wholth

#endif // WHOLTH_PAGE_H_
