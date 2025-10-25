#ifndef WHOLTH_PAGES_INGREDIENT_H_
#define WHOLTH_PAGES_INGREDIENT_H_

#include "sqlw/statement.hpp"
#include "wholth/c/entity/ingredient.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/pagination.hpp"

namespace wholth::pages
{

struct IngredientQuery
{
    std::string food_id{""};
    std::string title{""};
};

struct Ingredient
{
    IngredientQuery query{};
    wholth::model::SwappableBufferViewsAwareContainer<wholth_Ingredient>
        container{};
};

auto prepare_ingredient_stmt(
    sqlw::Statement& stmt,
    const IngredientQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>;

auto hydrate(Ingredient&, size_t index, wholth::entity::LengthContainer& lc)
    -> void;

} // namespace wholth::pages

#endif // WHOLTH_PAGES_INGREDIENT_H_
