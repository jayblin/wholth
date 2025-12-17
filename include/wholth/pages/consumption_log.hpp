#ifndef WHOLTH_PAGES_CONSUMPTION_LOG_H_
#define WHOLTH_PAGES_CONSUMPTION_LOG_H_

#include "sqlw/statement.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/c/entity/consumption_log.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/pagination.hpp"

namespace wholth::pages
{

struct ConsumptionLogQuery
{
    std::string user_id{""};
    std::string created_from{""};
    std::string created_to{""};
};

struct ConsumptionLog
{
    ConsumptionLogQuery query{};
    wholth::BufferView<std::vector<wholth_ConsumptionLog>> container{};
};

auto prepare_consumption_log_stmt(
    sqlw::Statement& stmt,
    const ConsumptionLogQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>;

auto hydrate(ConsumptionLog&, size_t index, wholth::entity::LengthContainer& lc)
    -> void;

} // namespace wholth::pages

#endif // WHOLTH_PAGES_CONSUMPTION_LOG_H_
