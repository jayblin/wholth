#ifndef WHOLTH_UTILS_IS_VALID_ID_H_
#define WHOLTH_UTILS_IS_VALID_ID_H_

#include <string_view>

namespace wholth::utils
{

auto is_valid_id(std::string_view id) -> bool;

} // namespace wholth::utils

#endif // WHOLTH_UTILS_IS_VALID_ID_H_
