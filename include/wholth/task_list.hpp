#ifndef WHOLTH_TASK_LIST_H_
#define WHOLTH_TASK_LIST_H_

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
    TaskList()
    {
    }

    TaskList(Task t)
    {
        add(t);
    }

    auto add(Task t) -> void
    {
        m_tasks_mask |= t;
    }

    auto add(const TaskList& tl) -> void
    {
        m_tasks_mask |= tl.mask();
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

    auto mask() const -> std::underlying_type_t<Task>
    {
        return m_tasks_mask;
    }

  protected:
    std::underlying_type_t<Task> m_tasks_mask{0};
};
} // namespace wholth

#endif // WHOLTH_TASK_LIST_H_
