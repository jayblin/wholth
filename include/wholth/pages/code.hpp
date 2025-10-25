#ifndef WHOLTH_PAGES_CODE_H_
#define WHOLTH_PAGES_CODE_H_

#include <cstdint>
#include <system_error>

namespace wholth::pages
{

enum class Code : int8_t
{
    OK = 0,
    INVALID_LOCALE_ID,
    SPAN_SIZE_TOO_BIG,
    QUERY_PAGE_TOO_BIG,
    QUERY_OFFSET_TOO_BIG,
    NOT_FOUND,
};

std::error_code make_error_code(Code);

} // namespace wholth::pages

namespace std
{

template <>
struct is_error_code_enum<wholth::pages::Code> : true_type
{
};

} // namespace std

#endif // WHOLTH_PAGES_CODE_H_
