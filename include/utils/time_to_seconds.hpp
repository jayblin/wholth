#ifndef UTILS_TIME_TO_SECONDS_H_
#define UTILS_TIME_TO_SECONDS_H_

#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

namespace utils
{

std::error_code time_to_seconds(
    std::string_view time,
    std::string& result_seconds);

} // namespace utils

namespace utils
{

enum class time_to_seconds_Code : int8_t
{
    OK = 0,
    INVALID_TIME_FORMAT,
    UNITLESS_VALUE,
    VALUE_IS_TOO_LARGE,
    EMPTY_VALUE,
};

std::error_code make_error_code(time_to_seconds_Code);

}; // namespace utils

namespace std
{

template <>
struct is_error_code_enum<utils::time_to_seconds_Code> : true_type
{
};

} // namespace std

#endif // UTILS_TIME_TO_SECONDS_H_
