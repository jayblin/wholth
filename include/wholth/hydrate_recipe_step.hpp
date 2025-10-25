#ifndef WHOLTH_HYDRATE_RECIPE_STEP_H_
#define WHOLTH_HYDRATE_RECIPE_STEP_H_

#include "wholth/hydrate_forward.hpp"

namespace wholth::concepts
{

template <typename T>
concept is_recipe_step = requires(T t) {
    t.id;
    t.time;
    t.description;
};

} // namespace wholth::concepts

namespace wholth
{

template <wholth::concepts::is_recipe_step T>
constexpr auto count_fields() -> size_t
{
    return 3;
}

template <wholth::concepts::is_recipe_step T>
auto hydrate(const std::string& buffer, wholth::utils::LengthContainer& lc) -> T
{
    static_assert(count_fields<T>() == 3);

    T entry;

    entry.id = lc.next<decltype(entry.id)>(buffer);
    entry.time = lc.next<decltype(entry.time)>(buffer);
    entry.description = lc.next<decltype(entry.description)>(buffer);

    return entry;
}

}; // namespace wholth

#endif // WHOLTH_HYDRATE_RECIPE_STEP_H_
