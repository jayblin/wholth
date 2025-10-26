#ifndef WHOLTH_UTILS_TO_ERROR_H_
#define WHOLTH_UTILS_TO_ERROR_H_

#include "wholth/c/error.h"
#include <cstdint>
#include <string_view>

namespace wholth::utils
{

constexpr wholth_Error to_error(int64_t code, const std::string_view sv)
{
    return {.code = code, .message = {.data = sv.data(), .size = sv.size()}};
}

} // namespace wholth::utils

#endif // WHOLTH_UTILS_TO_ERROR_H_
