#ifndef WHOLTH_UTILS_STR_SPLIT_H_
#define WHOLTH_UTILS_STR_SPLIT_H_

#include <string_view>
#include <vector>

namespace wholth::utils
{

std::vector<std::string_view> str_split(
    std::string_view str,
    std::string_view needle);

} // namespace wholth::utils

#endif // WHOLTH_UTILS_STR_SPLIT_H_
