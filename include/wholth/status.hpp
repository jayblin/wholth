#ifndef WHOLTH_STATUS_H_
#define WHOLTH_STATUS_H_

#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include <cstdint>
#include <limits>
#include <system_error>
#include <tuple>

namespace wholth::status {
    enum class Code : int8_t
    {
        OK = 0,
        /* SQL_STATEMENT_ERROR, */
        ENTITY_NOT_FOUND,
        INVALID_LOCALE_ID,
        INVALID_FOOD_ID,
        EMPTY_FOOD_TITLE,
        UNCHANGED_FOOD_TITLE,
        UNCHANGED_FOOD_DESCRIPTION,
        INVALID_DATE,
        INVALID_MASS,
        SPAN_SIZE_TOO_BIG,
        QUERY_PAGE_TOO_BIG,
        QUERY_OFFSET_TOO_BIG,
    };

    enum class Condition : int8_t
    {
        OK = 0,
        ERROR,
    };

    std::error_code make_error_code(Code);
    std::error_condition make_error_condition(Condition);
}

namespace std
{
    template <>
    struct is_error_code_enum<wholth::status::Code> : true_type {};

    template <>
    struct is_error_condition_enum<wholth::status::Condition> : true_type {};
}


#endif // WHOLTH_STATUS_H_
