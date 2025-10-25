#ifndef WHOLTH_ENTITY_NUTRIENT_H_
#define WHOLTH_ENTITY_NUTRIENT_H_

#include <cstddef>

namespace wholth::entity
{

template <typename T>
concept is_nutrient = requires(T t) {
    t.id;
    t.title;
    t.value;
    t.unit;
    t.position;
};

template <is_nutrient T>
constexpr auto count_fields() -> size_t
{
    return 5;
}

} // namespace wholth::entity

#endif // WHOLTH_ENTITY_NUTRIENT_H_
