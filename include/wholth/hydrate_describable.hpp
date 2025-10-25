#ifndef WHOLTH_HYDRATE_DISCRIBABLE_H_
#define WHOLTH_HYDRATE_DISCRIBABLE_H_

#include "wholth/hydrate_forward.hpp"

// namespace wholth::concepts
// {
//
// template <typename T>
// concept is_describable = requires(T t) { t.description; };
//
// } // namespace wholth::concepts
//
// namespace wholth
// {
//
// template <wholth::concepts::is_describable T>
// constexpr auto count_fields() -> size_t
// {
//     return 1;
// }
//
// template <wholth::concepts::is_describable T>
// auto hydrate(const std::string& buffer, wholth::utils::LengthContainer& lc) -> T
// {
//     static_assert(count_fields<T>() == 1);
//
//     T entry;
//
//     entry.description = lc.next<decltype(entry.description)>(buffer);
//
//     return entry;
// }
//
// }; // namespace wholth

#endif // WHOLTH_HYDRATE_DISCRIBABLE_H_
