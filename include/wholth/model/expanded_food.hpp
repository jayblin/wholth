#ifndef WHOLTH_MODEL_EXPANDED_FOOD_H_
#define WHOLTH_MODEL_EXPANDED_FOOD_H_

#include "wholth/entity/food.hpp"
#include <span>

// todo: move to model namespace
namespace wholth::model
{
struct ExpandedFood
{
    ExpandedFood()
    {
        /* nutrients.reserve(500); */
    }

    bool should_show{false};
    wholth::entity::expanded::Food food;
    std::string food_buffer{""};
    std::array<wholth::entity::expanded::food::Nutrient, 500> nutrients{};
    /* std::vector<wholth::entity::expanded::food::Nutrient> nutrients{}; */
    std::string nutrients_buffer{""};
    std::array<wholth::entity::expanded::food::RecipeStep, 1> steps{};
    std::string steps_buffer{};
};
} // namespace wholth::model

#endif // WHOLTH_MODEL_EXPANDED_FOOD_H_
