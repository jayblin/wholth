#ifndef WHOLTH_EM_CONSUMPTION_LOG_H_
#define WHOLTH_EM_CONSUMPTION_LOG_H_

#include <cstdint>
#include <system_error>

namespace wholth::entity_manager::consumption_log
{

enum class Code : int8_t
{
    OK = 0,
    CONSUMPTION_LOG_NULL,
    CONSUMPTION_LOG_INVALID_ID,
    CONSUMPTION_LOG_INVALID_FOOD_ID,
    CONSUMPTION_LOG_INVALID_MASS,
    CONSUMPTION_LOG_INVALID_DATE,
};

std::error_code make_error_code(Code);

}; // namespace wholth::entity_manager::consumption_log

namespace std
{

template <>
struct is_error_code_enum<wholth::entity_manager::consumption_log::Code>
    : true_type
{
};

} // namespace std

#endif // WHOLTH_EM_CONSUMPTION_LOG_H_
