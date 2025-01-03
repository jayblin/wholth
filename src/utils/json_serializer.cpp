#include "utils/json_serializer.hpp"

constexpr const char obj_beg = '{';
constexpr const char obj_end = '}';
constexpr const char arr_beg = '[';
constexpr const char arr_end = ']';
constexpr const char delimiter = ',';
constexpr std::string_view tab = "  ";
constexpr const char nl = '\n';

bool utils::JsonSerializer::should_delimit()
{
    return m_depth == m_upstairs_neighbor_depth;
}

void utils::JsonSerializer::begin_object()
{
    m_output_stream << obj_beg;
}

void utils::JsonSerializer::end_object()
{
    m_output_stream << obj_end;
}

void utils::JsonSerializer::begin_array()
{
    m_output_stream << arr_beg;
}

void utils::JsonSerializer::end_array()
{
    m_output_stream << arr_end;
}

void utils::JsonSerializer::tabulate()
{
    for (int64_t i = 0; i < depth(); i++) {
        m_output_stream << tab;
    }
}

void utils::JsonSerializer::new_line()
{
    m_output_stream << nl;
}

void utils::JsonSerializer::delimit()
{
    m_output_stream << delimiter;
}

void utils::JsonSerializer::sanitize_string(std::string& str)
{
    std::replace(str.begin(), str.end(), '"', '\'');
}
