#ifndef UI_COMPONENTS_TIMER_H_
#define UI_COMPONENTS_TIMER_H_

#include <chrono>

namespace ui::components
{

    using namespace std::chrono_literals;

    class Timer
    {
    public:
        static constexpr std::chrono::milliseconds default_timeout = 300ms;

        auto tick(const std::chrono::duration<double>& delta) -> void
        {
            if (m_search_timeout > 0ms)
            {
                m_search_timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(delta);
            }
        }

        auto has_expired() -> bool
        {
            if (0ms > m_search_timeout)
            {
                m_search_timeout = 0ms;
                return true;
            }

            return false;
        }

        auto start(std::chrono::milliseconds&& timeout) -> void
        {
            m_search_timeout = timeout;
        }

        auto start() -> void
        {
            m_search_timeout = default_timeout;
        }

        auto count() const
        {
            return m_search_timeout.count();
        }

        auto ms() const -> const std::chrono::milliseconds&
        {
            return m_search_timeout;
        }
    private:
        std::chrono::milliseconds m_search_timeout {0ms};
    };
}

#endif // UI_COMPONENTS_TIMER_H_
