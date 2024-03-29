#include "wholth/utils.hpp"

std::string wholth::utils::current_time_and_date()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%dT%H:%M:%S");

	return ss.str();
}
