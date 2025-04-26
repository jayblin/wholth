#ifndef UI_COMPONENTS_TIMER_H_
#define UI_COMPONENTS_TIMER_H_

#include <chrono>

namespace utils
{

using namespace std::chrono_literals;

/* class Clock */
/* { */
/*   public: */
/*     typedef std::chrono::duration<double> delta_t; */
/*     typedef std::chrono::milliseconds timeout_t; */
/*     static constexpr timeout_t asap = -1ms; */

/*     Clock() */
/*     { */
/*     } */

/*     Clock(timeout_t timeout) */
/*     { */
/*         start(timeout); */
/*     } */

/*     auto tick(const delta_t& delta) -> void */
/*     { */
/*         if (m_timeout > 0ms) */
/*         { */
/*             m_timeout -= std::chrono::duration_cast<timeout_t>(delta); */
/*         } */
/*     } */

/*     auto has_expired() const -> bool */
/*     { */
/*         if (m_timeout < 0ms) */
/*         { */
/*             /1* m_timeout = 0ms; *1/ */
/*             return true; */
/*         } */

/*         return false; */
/*     } */

/*     auto start(timeout_t timeout) -> void */
/*     { */
/*         m_timeout = timeout; */
/*     } */

/*     auto count() const */
/*     { */
/*         return m_timeout.count(); */
/*     } */

/*     auto ms() const -> const timeout_t& */
/*     { */
/*         return m_timeout; */
/*     } */

/*   private: */
/*     timeout_t m_timeout{asap}; */
/* }; */

class Timer
{
  public:
    typedef std::chrono::duration<double> delta_t;
    typedef std::chrono::milliseconds timeout_t;
    static constexpr timeout_t asap = -1ms;

    Timer()
    {
    }

    Timer(timeout_t timeout)
    {
        start(timeout);
    }

    auto tick(const delta_t& delta) -> void
    {
        if (m_timeout > 0ms)
        {
            m_timeout -= std::chrono::duration_cast<timeout_t>(delta);
        }
    }

    auto has_expired() const -> bool
    {
        if (m_timeout < 0ms)
        {
            /* m_timeout = 0ms; */
            return true;
        }

        return false;
    }

    auto start(timeout_t timeout) -> void
    {
        m_timeout = timeout;
    }

    auto count() const
    {
        return m_timeout.count();
    }

    auto ms() const -> const timeout_t&
    {
        return m_timeout;
    }

  private:
    timeout_t m_timeout{asap};
};
} // namespace utils

#endif // UI_COMPONENTS_TIMER_H_
