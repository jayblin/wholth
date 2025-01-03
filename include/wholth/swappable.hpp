#ifndef WHOLTH_SWAPPABLE_H_
#define WHOLTH_SWAPPABLE_H_

#include "utils/serializer.hpp"
#include <array>

namespace wholth {
template <typename T>
class Swappable {
public:
    auto current() -> T& { return m_values[m_idx]; };
    auto view_current() const -> const T& { return m_values[m_idx]; };
    auto next() -> T& { return m_values[(m_idx + 1) % 2]; };
    auto view_next() const -> const T& { return m_values[(m_idx + 1) % 2]; };
    auto swap() -> void { m_idx = (m_idx + 1) % 2; };

    template <typename Serializer>
    auto serialize(Serializer& serializer) const noexcept -> void
    {
        serializer
            << NVP(m_values)
            << NVP(m_idx);
    }

private:
    std::array<T, 2> m_values;
    uint8_t m_idx { 0 };
};
}

#endif // WHOLTH_SWAPPABLE_H_
