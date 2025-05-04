#ifndef WHOLTH_HYDRATE
#define WHOLTH_HYDRATE

#include "wholth/utils.hpp"
#include <type_traits>

namespace wholth::concepts
{

template <typename T>
concept is_food = requires(T t) {
    t.id;
    t.title;
    t.preparation_time;
    t.top_nutrient;
};

template <typename T>
concept is_nutrient = requires(T t) {
    t.id;
    t.title;
    t.value;
    t.unit;
    t.position;
};

template <typename T>
concept is_describable = requires(T t) { t.description; };

} // namespace wholth::concepts

namespace wholth
{
template <typename T>
constexpr auto count_fields() -> size_t;

template <typename T>
auto hydrate(const std::string&, wholth::utils::LengthContainer&) -> T;

template <wholth::concepts::is_food T>
constexpr auto count_fields() -> size_t
{
    return 4;
}

template <wholth::concepts::is_food T>
auto hydrate(const std::string& buffer, wholth::utils::LengthContainer& lc) -> T
{
    T entry;

    entry.id = lc.next<decltype(entry.id)>(buffer);
    entry.title = lc.next<decltype(entry.title)>(buffer);
    entry.preparation_time = lc.next<decltype(entry.preparation_time)>(buffer);
    entry.top_nutrient = lc.next<decltype(entry.top_nutrient)>(buffer);

    return entry;
}

template <wholth::concepts::is_nutrient T>
constexpr auto count_fields() -> size_t
{
    return 5;
}

template <wholth::concepts::is_nutrient T>
auto hydrate(const std::string& buffer, wholth::utils::LengthContainer& lc) -> T
{
    T entry;

    entry.id = lc.next<decltype(entry.id)>(buffer);
    entry.title = lc.next<decltype(entry.title)>(buffer);
    entry.value = lc.next<decltype(entry.value)>(buffer);
    entry.unit = lc.next<decltype(entry.unit)>(buffer);
    entry.position = lc.next<decltype(entry.position)>(buffer);

    return entry;
}
}; // namespace wholth

#endif // WHOLTH_HYDRATE
