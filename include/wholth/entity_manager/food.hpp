#ifndef WHOLTH_EM_FOOD_H_
#define WHOLTH_EM_FOOD_H_

#include <cstdint>
#include <system_error>

namespace wholth::entity_manager::food
{

enum class Code : int8_t
{
    OK = 0,
    FOOD_INVALID_ID,
    FOOD_NULL,
    FOOD_NULL_TITLE,
    FOOD_NULL_DESCRIPTION,
    // NULL_DETAILS,
};

std::error_code make_error_code(Code);

}; // namespace wholth::entity_manager::food

namespace std
{

template <>
struct is_error_code_enum<wholth::entity_manager::food::Code> : true_type
{
};

} // namespace std

#endif // WHOLTH_EM_FOOD_H_
