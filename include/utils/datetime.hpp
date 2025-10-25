#ifndef UTILS_DATETIME_H_
#define UTILS_DATETIME_H_

#include <string_view>

namespace utils::datetime
{
    auto is_valid_sqlite_datetime(std::string_view) -> bool;
}

#endif // UTILS_DATETIME_H_
