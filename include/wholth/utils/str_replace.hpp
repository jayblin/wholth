#ifndef WHOLTH_UTILS_STR_REPLACE_H_
#define WHOLTH_UTILS_STR_REPLACE_H_

#include <string>
#include <string_view>

namespace wholth::utils
{

std::string str_replace(
    std::string_view haystack,
    std::string_view needle,
    std::string_view replacement);

} // namespace wholth::utils

#endif // WHOLTH_UTILS_STR_REPLACE_H_
