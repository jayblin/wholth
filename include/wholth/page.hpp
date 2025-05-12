#ifndef WHOLTH_PAGE_H_
#define WHOLTH_PAGE_H_

#include <cstdint>
#include <string>
#include <string_view>

namespace wholth
{
// todo remove file
class Pagination
{
  public:
    using per_page_t = uint64_t;

    Pagination(uint64_t per_page) : m_per_page(per_page)
    {
    }

    // todo rename to `update_pagination()`?
    auto update() -> void;
    auto advance(uint64_t pages = 1) -> bool;
    auto retreat(uint64_t pages = 1) -> bool;
    auto per_page() const -> per_page_t
    {
        return m_per_page;
    };

    // todo rename or remove ?
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
    auto skip_to(uint64_t new_cur_page) -> bool;
    // todo test this
    auto max_page() const -> uint64_t
    {
        if (m_max_page > 0) {
            return m_max_page - 1;
        }
        return 0;
    }
    auto reset() -> void
    {
        m_count = 0;
        m_cur_page = 0;
        m_max_page = 1;
        m_pagination = "";
    }

  protected:
    uint64_t m_per_page{0};
    uint64_t m_count{0};
    uint64_t m_cur_page{0};
    uint64_t m_max_page{1};
    std::string m_pagination{""};
};
} // namespace wholth

#endif // WHOLTH_PAGE_H_
