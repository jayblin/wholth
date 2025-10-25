#include "utils/time_to_seconds.hpp"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>

std::error_code utils::time_to_seconds(
    std::string_view time,
    std::string& result_seconds)
{
    if (time.empty())
    {
        return utils::time_to_seconds_Code::EMPTY_VALUE;
    }

    constexpr static std::string_view valid_chars = "hms";
    constexpr static std::string_view skip_chars = " ";

    result_seconds = "";
    uint32_t seconds = 0;
    uint32_t i = 0;
    int32_t last_unit_pos = -1;
    size_t digit_count = 0;

    for (const auto ch : time)
    {
        i++;

        if (std::isdigit(ch))
        {
            digit_count++;
            continue;
        }

        const auto unit_of_time = valid_chars.find_first_of(ch);

        if (std::string_view::npos == unit_of_time)
        {
            if (std::string_view::npos != skip_chars.find_first_of(ch))
            {
                continue;
            }

            return utils::time_to_seconds_Code::INVALID_TIME_FORMAT;
        }

        size_t k = 0;
        uint32_t intermediate = 0;
        for (int32_t j = (i - 2); j > last_unit_pos; j--)
        {
            if (!std::isdigit(time[j]))
            {
                break;
            }

            intermediate += (time[j] - '0') * std::pow(10, k);
            digit_count--;

            k++;
        }

        uint32_t multiplier = 1;

        if ('h' == ch)
        {
            multiplier = 60 * 60;
        }
        else if ('m' == ch)
        {
            multiplier = 60;
        }

        const uint32_t tmp = intermediate * multiplier;

        if ((std::numeric_limits<uint32_t>::max() == intermediate &&
             std::numeric_limits<uint32_t>::max() == tmp) ||
            intermediate > tmp || seconds > (seconds + tmp))
        {
            return utils::time_to_seconds_Code::VALUE_IS_TOO_LARGE;
        }

        seconds += tmp;

        last_unit_pos = i - 1;
    }

    if (digit_count > 0)
    {
        return utils::time_to_seconds_Code::UNITLESS_VALUE;
    }

    if (0 == seconds && '0' != time[0])
    {
        return utils::time_to_seconds_Code::EMPTY_VALUE;
    }

    std::stringstream ss;
    ss << seconds;

    result_seconds = ss.str();

    return utils::time_to_seconds_Code::OK;
}

namespace utils
{

struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override final
    {
        return "time_to_seconds";
    }

    std::string message(int ev) const override final
    {
        using Code = time_to_seconds_Code;

        switch (static_cast<Code>(ev))
        {
        case Code::OK:
            return "no error";
        case Code::INVALID_TIME_FORMAT:
            return "input is badly formatted";
        case Code::UNITLESS_VALUE:
            return "input has a value without unit of time";
        case Code::VALUE_IS_TOO_LARGE:
            return "resulting number of seconds is so large that it overflowed "
                   "uint32 (136 years is not enogh for you?)";
        case Code::EMPTY_VALUE:
            return "EMPTY_VALUE";
        }

        return "(unrecognized error)";
    }
};

const ErrorCategory time_to_seconds_error_category{};
} // namespace utils

std::error_code utils::make_error_code(utils::time_to_seconds_Code e)
{
    return {static_cast<int>(e), utils::time_to_seconds_error_category};
}
