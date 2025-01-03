#ifndef WHOLTH_PAGE_H_
#define WHOLTH_PAGE_H_

#include "utils/serializer.hpp"
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
    auto count() -> uint64_t&
    {
        return m_count;
    };
    auto current_page() const -> uint64_t
    {
        return m_cur_page;
    };
    auto current_page(uint64_t new_cur_page) -> void
    {
        m_cur_page = new_cur_page;
    };

    template <typename Serializer>
    auto serialize(Serializer& serializer) const noexcept -> void
    {
        serializer << NVP(m_count) << NVP(m_cur_page) << NVP(m_max_page)
                   << NVP(m_pagination);
    }

  private:
    uint64_t m_per_page{0};
    uint64_t m_count{0};
    uint64_t m_cur_page{0};
    uint64_t m_max_page{0};
    std::string m_pagination{""};
};
} // namespace wholth

#endif // WHOLTH_PAGE_H_