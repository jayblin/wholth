#ifndef WHOLTH_PAGE_H_
#define WHOLTH_PAGE_H_

#include <cstdint>

namespace wholth
{
class Pagination
{
  public:
    using per_page_t = uint64_t;

    Pagination()
    {
    }

    Pagination(uint64_t per_page) : m_per_page(per_page)
    {
    }

    Pagination(Pagination&&) = default;
    Pagination& operator=(Pagination&&) = default;

    // todo rename to `update_pagination()`?
    auto update() -> void;
    auto advance(uint64_t pages = 1) -> bool;
    auto retreat(uint64_t pages = 1) -> bool;
    auto per_page() const -> per_page_t
    {
        return m_per_page;
    };
    auto per_page(per_page_t value) -> void
    {
        reset();
        m_per_page = value;
    };

    // todo rename or remove ?
    // auto pagination() const -> std::string_view
    // {
    //     return m_pagination;
    // };
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
    /*
     * Returns max page zero indexed.
     */
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
        m_span_size = 0;
        m_cur_page = 0;
        m_max_page = 1;
        // m_pagination = "";
    }
    auto span_size() const -> uint64_t
    {
        return m_span_size;
    }
    auto span_size(uint64_t size) -> void
    {
        m_span_size = size;
    }

  protected:
    // Hom many elements per page.
    per_page_t m_per_page{0};
    // Hom many elements across all pages.
    uint64_t m_count{0};
    // Hom many elements are on current page.
    uint64_t m_span_size{0};
    // Number of current page - zero based.
    uint64_t m_cur_page{0};
    // Number of the last page - zero based.
    uint64_t m_max_page{1};
    // Pagination string in form of {cur_page}/{last_page}.
    // todo remove? as it apears to be not needed.
    // std::string m_pagination{""};
};
} // namespace wholth

#endif // WHOLTH_PAGE_H_
