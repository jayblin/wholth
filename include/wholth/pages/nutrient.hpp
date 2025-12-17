#ifndef WHOLTH_PAGES_NUTRIENT_H_
#define WHOLTH_PAGES_NUTRIENT_H_

#include "sqlw/statement.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/c/entity/nutrient.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/pagination.hpp"

namespace wholth::pages
{

struct NutrientQuery
{
    std::string title{""};
};

struct Nutrient
{
    NutrientQuery query{};
    wholth::BufferView<std::vector<wholth_Nutrient>> container{};
};

auto prepare_nutrient_stmt(
    sqlw::Statement& stmt,
    const NutrientQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>;

auto hydrate(Nutrient&, size_t index, wholth::entity::LengthContainer& lc)
    -> void;

} // namespace wholth::pages

#endif // WHOLTH_PAGES_NUTRIENT_H_
