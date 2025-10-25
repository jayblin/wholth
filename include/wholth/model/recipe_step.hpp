#ifndef WHOLTH_MODEL_RECIEP_STEP_H_
#define WHOLTH_MODEL_RECIEP_STEP_H_

#include "wholth/context.hpp"
#include "wholth/hydrate_recipe_step.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/page.hpp"

namespace wholth::model
{

struct RecipeStep
{
    const wholth::Context& ctx;
    std::atomic<bool> is_fetching{false};
    std::string food_id{""};
};

template <wholth::concepts::is_recipe_step T>
using RecipeStepContainer = wholth::Swappable<wholth::BufferView<T>>;

} // namespace wholth::model

#endif // WHOLTH_MODEL_RECIEP_STEP_H_
