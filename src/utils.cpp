#include "wholth/utils.hpp"
#include <chrono>
#include <system_error>

// todo move to date namesapce
auto wholth::utils::current_time_and_date() -> std::string
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%dT%H:%M:%S");

    /* auto now = std::chrono::system_clock::now(); */
    /* std::time_t timestamp = std::chrono::system_clock::to_time_t(now); */
    /* std::tm* t = std::localtime(&timestamp); */
    /* fmt::print("{}-{}-{} {}\n", 1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday,
     * t->tm_hour); */

    return ss.str();
}

// todo move to other file
auto wholth::utils::sqlite::seconds_to_readable_time(
    sqlite3_context* ctx,
    int argc,
    sqlite3_value** argv) -> void
{
    if (1 != argc || sqlite3_value_type(argv[0]) != SQLITE_INTEGER)
    {
        return;
    }

    int seconds = sqlite3_value_int(argv[0]);
    std::stringstream ss;
    if (seconds < 60)
    {
        ss << seconds << 's';
    }
    else if (seconds < 3600)
    {
        ss.precision(0);
        ss << seconds / 60 << 'm';
    }
    else
    {
        ss.precision(0);
        auto hours = seconds / 3600;
        auto minutes = (seconds % 3600) / 60;
        ss << hours << 'h';

        if (minutes > 0)
        {
            ss << ' ' << minutes << 'm';
        }
    }

    auto str = ss.str();

    sqlite3_result_text(ctx, str.data(), str.size(), SQLITE_TRANSIENT);
}
