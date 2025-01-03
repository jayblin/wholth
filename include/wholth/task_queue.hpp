#ifndef WHOLTH_TASK_QUEUE_H_
#define WHOLTH_TASK_QUEUE_H_

#include "utils/serializer.hpp"
#include "wholth/entity/food.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace wholth
{
enum Task : uint16_t
{
    /* _FIRST_ = 0, */
    /* CHANGED_LOCALE, */
    FETCH_FOODS = 0,
    FETCH_EXPANDED_FOOD_INFO,
    _LAST_,
};

// @todo
// - add bounds checking?
// - rename cause i need it to mean something like 'ThreadedTaskQueue'.
class TaskQueue
{
  public:
    typedef uint16_t event_t;
    typedef std::array<uint8_t, Task::_LAST_> queue_t;
    /* typedef std::tuple< */
    /*     nullptr_t,                 // */
    /*     wholth::entity::food::id_t // */
    /*     > */
    /*     data_t; */

    auto enqueue(Task t) -> void
    {
        m_queue[t] = m_queue[t] + 1;
        m_size++;
    }
    auto dequeue(Task t) -> void
    {
        if (m_queue[t] > 0)
        {
            m_queue[t] = m_queue[t] - 1;
            m_size--;
        }
    }
    auto queue() const -> const queue_t&
    {
        return m_queue;
    };
    /* auto data() const -> const data_t& */
    /* { */
    /*     return m_data; */
    /* } */
    auto is_empty() const -> bool
    {
        return m_size == 0;
    }

    template <typename Serializer>
    auto serialize(Serializer& serializer) const noexcept -> void
    {
        serializer << NVP(m_queue) //
                   << NVP(m_size)  //
            ;
    }

  private:
    queue_t m_queue{};
    /* data_t m_data{}; */
    uint32_t m_size{0};
};
/* constexpr auto a = sizeof(TaskQueue); */
} // namespace wholth

#endif // WHOLTH_TASK_QUEUE_H_
