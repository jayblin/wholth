#include "wholth/utils/prepend_sql_params.hpp"
#include "fmt/core.h"
#include "wholth/utils/to_string_view.hpp"

auto wholth::utils::prepend_sql_params(
    const std::span<const extended_param_t>& param_map,
    std::vector<sqlw::statement::internal::bindable_t>& params,
    std::stringstream& params_template_stream) -> void
{
    // todo add constexpr test for size() <> capacity().
    size_t idx = 0;
    for (const auto& p : param_map)
    {
        if (nullptr == std::get<1>(p).data || std::get<1>(p).size <= 0)
        {
            continue;
        }
        idx++;

        // if (params.size() > 2)
        if (idx > 1)
        {
            params_template_stream << ", ";
        }

        params.emplace_back(to_string_view(std::get<1>(p)), std::get<2>(p));

        if (std::get<3>(p).empty())
        {
            params_template_stream
                << fmt::format("{} = ?{}", std::get<0>(p), params.size());
        }
        else
        {
            params_template_stream << fmt::format(
                "{} = {}",
                std::get<0>(p),
                fmt::format(fmt::runtime(std::get<3>(p)), params.size()));
        }
    }
}
