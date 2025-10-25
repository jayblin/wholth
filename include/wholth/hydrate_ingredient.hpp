#ifndef WHOLTH_HYDRATE_INGREDIENT_H_
#define WHOLTH_HYDRATE_INGREDIENT_H_

#include "wholth/hydrate_forward.hpp"

namespace wholth::concepts
{

template <typename T>
concept is_ingredient = requires(T t) {
    t.food_id;
    t.food_title;
    t.canonical_mass_g;
    t.ingredient_count;
};

} // namespace wholth::concepts

namespace wholth
{

template <wholth::concepts::is_ingredient T>
constexpr auto count_fields() -> size_t
{
    return 4;
}

template <wholth::concepts::is_ingredient T>
auto hydrate(const std::string& buffer, wholth::utils::LengthContainer& lc) -> T
{
    static_assert(count_fields<T>() == 4);

    T entry;

    entry.food_id = lc.next<decltype(entry.food_id)>(buffer);
    entry.food_title = lc.next<decltype(entry.food_title)>(buffer);
    entry.canonical_mass_g = lc.next<decltype(entry.canonical_mass_g)>(buffer);
    entry.ingredient_count = lc.next<decltype(entry.ingredient_count)>(buffer);

    return entry;
}

}; // namespace wholth

#endif // WHOLTH_HYDRATE_INGREDIENT_H_
