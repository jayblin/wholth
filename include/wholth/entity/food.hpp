#ifndef WHOLTH_ENTITY_FOOD_H_
#define WHOLTH_ENTITY_FOOD_H_

#include <type_traits>

namespace wholth::entity
{

template <typename T>
concept is_food = requires(T t) {
    t.id;
    t.title;
    t.preparation_time;
    t.top_nutrient;
};

template <is_food T>
constexpr auto count_fields() -> size_t
{
    return 5;
}

} // namespace wholth::entity

#endif // WHOLTH_ENTITY_FOOD_H_
