#ifndef WHOLTH_PAGES_FOOD_NUTRIENT_H_
#define WHOLTH_PAGES_FOOD_NUTRIENT_H_

#include "sqlw/statement.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/c/entity/nutrient.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/pagination.hpp"

namespace wholth::pages
{

struct FoodNutrientQuery
{
    std::string food_id{""};
    std::string title{""};
    std::string locale_id{""};
};

struct FoodNutrient
{
    FoodNutrientQuery query{};
    wholth::BufferView<std::vector<wholth_Nutrient>> container{};
};

auto prepare_food_nutrient_stmt(
    sqlw::Statement& stmt,
    const FoodNutrientQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>;

auto hydrate(
    // const std::string& buffer,
    FoodNutrient&,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void;

} // namespace wholth::pages

#endif // WHOLTH_PAGES_FOOD_NUTRIENT_H_
