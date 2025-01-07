#ifndef WHOLTH_TASK_LIST_H_
#define WHOLTH_TASK_LIST_H_

#include "utils/serializer.hpp"
#include <cstdint>
#include <type_traits>

namespace wholth
{
enum Task : uint8_t
{
    FETCH_FOODS = 0b0001,
    FETCH_EXPANDED_FOOD_INFO = 0b0010,
};

class TaskList
{
  public:
    auto add(Task t) -> void
    {
        m_tasks_mask |= t;
    }

    auto remove(Task t) -> void
    {
        m_tasks_mask ^= t;
    }

    auto is_empty() const -> bool
    {
        return 0 == m_tasks_mask;
    }

    auto clear() -> void
    {
        m_tasks_mask = 0;
    }

    auto has(Task t) -> bool
    {
        return t == (m_tasks_mask & t);
    }

    template <typename Serializer>
    auto serialize(Serializer& serializer) const noexcept -> void
    {
        serializer << NVP(m_tasks_mask) //
            ;
    }

  protected:
    std::underlying_type_t<Task> m_tasks_mask{0};
};
} // namespace wholth

#endif // WHOLTH_TASK_LIST_H_
