#ifndef WHOLTH_EM_INGREDIENT_H_
#define WHOLTH_EM_INGREDIENT_H_

#include <cstdint>
#include <system_error>

namespace wholth::entity_manager::ingredient
{

enum class Code : int8_t
{
    OK = 0,
    INGREDIENT_INVALID_ID,
    INGREDIENT_INVALID_FOOD_ID,
    INGREDIENT_INVALID_MASS,
    INGREDIENT_POSTCONDITION_FAILED,
    INGREDIENT_IS_NULL,
};

std::error_code make_error_code(Code);

}; // namespace wholth::entity_manager::ingredient

namespace std
{

template <>
struct is_error_code_enum<wholth::entity_manager::ingredient::Code> : true_type
{
};

} // namespace std

#endif // WHOLTH_EM_INGREDIENT_H_
