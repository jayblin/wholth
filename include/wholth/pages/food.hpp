#ifndef WHOLTH_PAGES_FOOD_H_
#define WHOLTH_PAGES_FOOD_H_

#include "sqlw/statement.hpp"
#include "wholth/c/entity/food.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/model/abstract_page.hpp"

namespace wholth::pages
{

struct FoodQuery
{
    std::string title{""};
    std::string ingredients{""};
};

struct Food
{
    int64_t slot{-1};
    FoodQuery query{};
    wholth::model::SwappableBufferViewsAwareContainer<wholth_Food> container{};
};

auto prepare_food_stmt(
    sqlw::Statement& stmt,
    const FoodQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>;

auto hydrate(
    // const std::string& buffer,
    Food&,
    size_t index,
    wholth::entity::LengthContainer&) -> void;

// template <>
// constexpr auto count_fields<wholth_Food>() -> size_t
// {
//     static_assert(wholth::entity::is_food<wholth_Food>);
//
//     return 4;
// }

} // namespace wholth::pages::food

#endif // WHOLTH_PAGES_FOOD_H_
