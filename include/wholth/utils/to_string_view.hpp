#ifndef WHOLTH_UTILS_TO_STRING_VIEW_H_
#define WHOLTH_UTILS_TO_STRING_VIEW_H_

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>

namespace wholth::utils
{

// todo add tests
template <typename T>
constexpr std::string_view to_string_view(const T& sv)
{
    if constexpr (std::same_as<std::remove_cvref_t<T>, std::string_view>)
    {
        return sv;
    }
    else if constexpr (std::same_as<std::remove_cvref_t<T>, std::string>)
    {
        return {sv.data(), sv.size()};
    }
    else
    {
        return {sv.data, sv.size};
    }
}

} // namespace wholth::utils

#endif // WHOLTH_UTILS_TO_STRING_VIEW_H_
