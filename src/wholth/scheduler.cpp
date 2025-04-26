#include "wholth/scheduler.hpp"
#include "wholth/task_list.hpp"
#include <type_traits>

/* bool wholth::Scheduler::is_empty() const */
/* { */
/*     return m_reserved_slots >= m_list.size(); */
/* } */
/* typedef std::array< */
/*     Timer::timeout_t, */
/*     static_cast<std::underlying_type_t<wholth::schedule::Group>>( */
/*         wholth::schedule::Group::__LAST__)> */
/*     timemap_t; */

/* static constexpr timemap_t timemap() */
/* { */
/*     constexpr timemap_t map{}; */

/*     map[wholth::schedule::Group::USER_INPUT] = 300ms; */

/*     return map; */
/* } */

static constexpr Timer::timeout_t timemap(
    wholth::schedule::Group group = wholth::schedule::Group::__LAST__)
{
    using G = wholth::schedule::Group;

    switch (group)
    {
    case G::USER_INPUT:
        return 300ms;
    default:
        return -1ms;
    }
}

void wholth::Scheduler::schedule(
    Task t,
    /* Timer::timeout_t timeout, */
    group_t group)
{
    using G = wholth::schedule::Group;

    /* if (group != G::__) */
    {
        auto& entry = m_list[static_cast<std::underlying_type_t<G>>(group)];
        if (!std::get<TaskList>(entry).has(t))
        {
            std::get<TaskList>(entry).add(t);
        }

        /* std::get<Timer>(entry).start(timeout); */
        std::get<Timer>(entry).start(timemap(group));

        return;
    }

    /*     if (m_reserved_slots >= m_list.size()) */
    /*     { */
    /*         // @todo Warn app of this condition. */
    /*         return; */
    /*     } */

    /*     for (size_t i = 0; i < m_list.size(); i++) */
    /*     { */
    /*         if (std::get<Timer>(m_list[i]).has_expired()) */
    /*         { */
    /*             m_list[i] = entry_t{t, timeout}; */
    /*             m_reserved_slots++; */
    /*             break; */
    /*         } */
    /*     } */
}

wholth::TaskList wholth::Scheduler::get_due()
{
    /* if (0 == m_reserved_slots) */
    /* { */
    /*     return {}; */
    /* } */

    wholth::TaskList result;

    for (size_t i = 0; i < m_list.size(); i++)
    {
        const auto& timer = std::get<Timer>(m_list[i]);
        auto& task_list = std::get<TaskList>(m_list[i]);

        if (!task_list.is_empty() && timer.has_expired())
        {
            result.add(task_list);
            task_list.clear();
            /* m_reserved_slots--; */
            break;
        }
    }

    return result;
}

void wholth::Scheduler::tick(const Timer::delta_t& delta)
{
    for (auto& e : m_list)
    {
        std::get<Timer>(e).tick(delta);
    }
}
