#ifndef WHOLTH_EM_NUTRIENT_H_
#define WHOLTH_EM_NUTRIENT_H_

#include <cstdint>
#include <system_error>

namespace wholth::entity_manager::nutrient
{

enum class Code : int8_t
{
    OK = 0,
    NUTRIENT_INVALID_ID,
    NUTRIENT_INVALID_VALUE,
    NUTRIENT_NULL,
};

std::error_code make_error_code(Code);

}; // namespace wholth::controller

namespace std
{

template <>
struct is_error_code_enum<wholth::entity_manager::nutrient::Code>
    : true_type
{
};

} // namespace std

#endif // WHOLTH_EM_NUTRIENT_H_

