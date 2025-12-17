#ifndef WHOLTH_SWAPPABLE_H_
#define WHOLTH_SWAPPABLE_H_

#include <array>

namespace wholth::concepts
{
    template <typename T>
    concept is_swappable = requires (T t)
    {
        /* T::value_t; */
        // t.next();
        // t.swap();
        t.current();
        t.view_current();
        t.next();
        t.view_next();
        t.swap();
        t.values();
        t.current_index();
    };
}

namespace wholth
{
template <typename T>
class Swappable
{
  public:
    using value_t = T;

    Swappable()
    {
    }
    Swappable(std::array<T, 2>&& values) : m_values(values)
    {
    }

    auto current() -> T&
    {
        return m_values[m_idx];
    };
    auto view_current() const -> const T&
    {
        return m_values[m_idx];
    };
    auto next() -> T&
    {
        return m_values[(m_idx + 1) % 2];
    };
    auto view_next() const -> const T&
    {
        return m_values[(m_idx + 1) % 2];
    };
    auto swap() -> void
    {
        m_idx = (m_idx + 1) % 2;
    };
    auto values() const -> const std::array<T, 2>&
    {
        return m_values;
    }
    auto current_index() const -> uint8_t
    {
        return m_idx;
    }

  private:
    std::array<T, 2> m_values;
    uint8_t m_idx{0};
};
static_assert(wholth::concepts::is_swappable<Swappable<int>>);

} // namespace wholth

#endif // WHOLTH_SWAPPABLE_H_
