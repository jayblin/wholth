#ifndef WHOLTH_PAGES_RECIPE_STEP_H_
#define WHOLTH_PAGES_RECIPE_STEP_H_

#include "sqlw/statement.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/c/entity/recipe_step.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/pagination.hpp"

namespace wholth::pages
{

struct RecipeStepQuery
{
    std::string recipe_id{""};
};

struct RecipeStep
{
    RecipeStepQuery query{};
    wholth::BufferView<std::vector<wholth_RecipeStep>> container{};
};

auto prepare_recipe_step_stmt(
    sqlw::Statement& stmt,
    const RecipeStepQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>;

auto hydrate(
    RecipeStep&,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void;

} // namespace wholth::pages

#endif // WHOLTH_PAGES_RECIPE_STEP_H_
