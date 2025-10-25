#ifndef WHOLTH_UTILS_PREPEND_SQL_PARAMS_H_
#define WHOLTH_UTILS_PREPEND_SQL_PARAMS_H_

#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/forward.h"
#include <span>
#include <sstream>
#include <string_view>
#include <tuple>
#include <vector>

namespace wholth::utils
{

using extended_param_t = std::tuple<
    std::string_view, // column_name
    const wholth_StringView&, // value
    sqlw::Type, // type
    std::string_view>; // wrapper_function

auto prepend_sql_params(
    const std::span<const extended_param_t>& param_map,
    std::vector<sqlw::statement::internal::bindable_t>& params,
    std::stringstream& params_template_stream) -> void;

} // namespace wholth::utils

#endif // WHOLTH_UTILS_PREPEND_SQL_PARAMS_H_
