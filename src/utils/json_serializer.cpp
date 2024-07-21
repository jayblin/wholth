#include "utils/json_serializer.hpp"

constexpr const char obracket = '{';
constexpr const char obj_beg = '{';
constexpr const char obj_end = '}';
constexpr const char arr_beg = '[';
constexpr const char arr_end = ']';
constexpr const char delimiter = ',';
constexpr std::string_view tab = "  ";
constexpr const char nl = '\n';

bool utils::JsonSerializer::should_delimit()
{
    auto buf_size = m_output_stream.rdbuf()->in_avail();

    if (buf_size < 1)
    {
        return false;
    }

    const auto g = m_output_stream.tellg();

    if (buf_size < 2)
    {
        const char ch = m_output_stream.get();

        m_output_stream.seekg(g);

        return ch != obracket;
    }

    m_output_stream.seekg(-1, m_output_stream.end);

    const char ch1 = m_output_stream.get();

    m_output_stream.seekg(-2, m_output_stream.end);

    const char ch2 = m_output_stream.get();

    m_output_stream.seekg(g);

    return ch2 != obracket || ch1 != nl;
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
