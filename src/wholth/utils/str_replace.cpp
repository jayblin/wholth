#include "wholth/utils/str_replace.hpp"
#include <sstream>
#include <cassert>

// todo refactor and test. there are edge cases to be.
std::string wholth::utils::str_replace(
    std::string_view haystack,
    std::string_view needle,
    std::string_view replacement)
{
    if (needle.empty())
    {
        return std::string{haystack};
    }

    assert(needle.size() == 1 && "what u want is not realized yet!");

    std::stringstream ss{};
    auto flag = 0;

    size_t j = 0;
    for (size_t i = 0; i < haystack.size(); i++)
    {
        if (needle[0] == haystack[i])
        {
            if (flag)
            {
                ss << replacement;
            }

            ss << std::string_view{haystack.data() + j, i - j};
            j = i + 1;

            flag = true;
        }

        if (i == haystack.size() - 1 && flag)
        {
            ss << replacement
               << std::string_view{haystack.data() + j, i - j + 1};
        }
    }

    return flag ? ss.str() : std::string{haystack};
}
