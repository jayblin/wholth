#include "wholth/utils/str_split.hpp"
#include <sstream>

std::vector<std::string_view> wholth::utils::str_split(
    std::string_view str,
    std::string_view needle)
{
    std::vector<std::string_view> parts{};
    std::stringstream ss{};
    size_t j = 0;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (needle[0] == str[i])
        {
            // if (j > 0)
            // {
            //     ss << needle_replacement;
            // }

            parts.emplace_back(str.data() + j, i - j);

            j = i + 1;
        }
    }

    // if (j > 0)
    // {
    //     ss << needle_replacement;
    // }

    parts.emplace_back(str.data() + j, str.size() - j);
    // ss << "?" << 5 + j;

    return parts;
}
