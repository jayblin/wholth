#ifndef WHOLTH_SCHEDULER_H_
#define WHOLTH_SCHEDULER_H_

#include "utils/timer.hpp"
#include "wholth/task_list.hpp"
#include <array>
#include <span>
#include <tuple>
#include <type_traits>
#include <vector>

using namespace std::chrono_literals;
using Timer = ::utils::Timer;

namespace wholth
{
    namespace schedule
    {
        enum class Group : size_t
        {
            USER_INPUT = 0,
            __LAST__,
        };
    }

class Scheduler
{
  public:
    /* Scheduler() */
    /* { */
    /* } */
    /* Scheduler(){} */

    typedef std::tuple<wholth::TaskList, Timer> entry_t;
    /* typedef size_t group_t; */
    typedef schedule::Group group_t;
    /* static constexpr size_t max_entries = schedule::Group::__LAST__; */
    static constexpr auto max_entries = static_cast<std::underlying_type_t<schedule::Group>>(schedule::Group::__LAST__);

    auto schedule(
        wholth::Task,
        /* Timer::timeout_t = Timer::asap, */
        group_t = schedule::Group::__LAST__) -> void;
    /* auto schedule(wholth::Task, group_t) -> void; */
    /* void evict_past(::utils::Timer::timeout_t); */
    auto get_due() -> wholth::TaskList;
    /* auto is_empty() const -> bool; */
    auto tick(const Timer::delta_t&) -> void;

  private:
    /* std::vector<entry_t> m_list; */
    /* size_t m_reserved_slots{0}; */
    std::array<entry_t, max_entries> m_list{};
};
} // namespace wholth

#endif // WHOLTH_SCHEDULER_H_
