#include "utils/datetime.hpp"
#include "utils/convert.hpp"

auto utils::datetime::is_valid_sqlite_datetime(std::string_view sv) -> bool
{
    //                                             111111111
    //                                   0123456789012345678
    constexpr std::string_view format = "YYYY-MM-DDTHH:MM:SS";

    if (sv.size() != format.size())
    {
        return false;
    }

    unsigned int year{0};
    auto ec = utils::convert::to_uint(sv.substr(0, 4), year);

    if (ec)
    {
        return false;
    }

    const std::chrono::year cyear{static_cast<int>(year)};

    if (!cyear.ok())
    {
        return false;
    }

    if ('-' != sv[4])
    {
        return false;
    }

    unsigned int month{0};
    ec = utils::convert::to_uint(sv.substr(5, 2), month);

    if (ec)
    {
        return false;
    }

    const std::chrono::month cmonth{month};

    if (!cmonth.ok())
    {
        return false;
    }

    if ('-' != sv[7])
    {
        return false;
    }

    unsigned int day{0};
    ec = utils::convert::to_uint(sv.substr(8, 2), day);

    if (ec)
    {
        return false;
    }

    const std::chrono::day cday{day};

    if (!cday.ok())
    {
        return false;
    }

    const std::chrono::year_month_day ymd{cyear, cmonth, cday};

    if (!ymd.ok())
    {
        return false;
    }

    if ('T' != sv[10])
    {
        return false;
    }

    unsigned int hour{0};
    ec = utils::convert::to_uint(sv.substr(11, 2), hour);

    if (ec || hour > 23)
    {
        return false;
    }

    if (':' != sv[13])
    {
        return false;
    }

    unsigned int minute{0};
    ec = utils::convert::to_uint(sv.substr(11, 2), minute);

    if (ec || minute > 59)
    {
        return false;
    }

    if (':' != sv[16])
    {
        return false;
    }

    unsigned int second{0};
    ec = utils::convert::to_uint(sv.substr(11, 2), second);

    if (ec || minute > 59)
    {
        return false;
    }

    return true;
}
