#include "wholth/utils/is_valid_id.hpp"
#include <algorithm>

bool wholth::utils::is_valid_id(std::string_view id)
{
    if (id.size() <= 0 || id.empty())
    {
        return false;
    }

    const auto iter = std::find_if_not(id.begin(), id.end(), std::isdigit);

    return iter == id.end();
}
