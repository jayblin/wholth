#include "wholth/utils.hpp"

std::string wholth::utils::current_time_and_date()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%dT%H:%M:%S");

	return ss.str();
}


void wholth::utils::sqlite::seconds_to_readable_time(
	sqlite3_context* ctx,
	int argc,
	sqlite3_value** argv
)
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
