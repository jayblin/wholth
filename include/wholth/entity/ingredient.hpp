#ifndef WHOLTH_ENTITY_INGREDIENT_H_
#define WHOLTH_ENTITY_INGREDIENT_H_

#include "wholth/entity/food.hpp"
#include <string_view>

namespace wholth::entity::ingredient
{

typedef std::string_view canonical_mass_t;
typedef std::string_view ingredient_count_t;

}; // namespace wholth::entity::ingredient

namespace wholth::entity
{

struct Ingredient
{
    std::string_view food_id{""};
    std::string_view food_title{""};
    ingredient::canonical_mass_t canonical_mass_g{""};
    ingredient::ingredient_count_t ingredient_count{""};
};

} // namespace wholth::entity

#endif // WHOLTH_ENTITY_INGREDIENT_H_
