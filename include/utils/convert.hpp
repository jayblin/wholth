#ifndef UTILS_CONVERT_H_
#define UTILS_CONVERT_H_

#include <concepts>
#include <string_view>
#include <system_error>

namespace utils::convert
{

template <std::unsigned_integral T>
auto to_uint(std::string_view sv, T& result) -> std::error_code
{
    result = 0;

    if (sv.size() == 0)
    {
        return std::make_error_code(std::errc::invalid_argument);
    }

    size_t exp = 1;
    T prev_result = 0;

    for (int64_t i = (sv.size() - 1); i >= 0; i--)
    {
        const auto ch = sv[i];

        if (!std::isdigit(ch))
        {
            result = 0;
            return std::make_error_code(std::errc::invalid_argument);
        }

        // todo check for overflow

        prev_result = result;

        uint64_t n = ch - '0';
        result += n * exp;

        if (prev_result > result)
        {
            result = 0;
            return std::make_error_code(std::errc::invalid_argument);
        }

        exp *= 10;
    }

    return {};
}

} // namespace utils::convert

#endif // UTILS_CONVERT_H_
