#ifndef WHOLTH_EM_RECIPE_STEP_H_
#define WHOLTH_EM_RECIPE_STEP_H_

#include <cstdint>
#include <system_error>

namespace wholth::entity_manager::recipe_step
{

enum class Code : int8_t
{
    OK = 0,
    RECIPE_STEP_INVALID_ID,
    RECIPE_STEP_NULL,
    RECIPE_STEP_NULL_TIME,
    RECIPE_STEP_ALREADY_EXISTS,
    RECIPE_STEP_NULL_DESCRIPTION,
    RECIPE_STEP_ID_MISMATCH,
    RECIPE_STEP_INSERT_FAILURE,
    RECIPE_STEP_INSERT_FAILURE_NO_RET_ID,
    RECIPE_STEP_UPDATE_FAILURE,
    RECIPE_STEP_LOCALISATION_UPDATE_FAILURE,
};

std::error_code make_error_code(Code);

}; // namespace wholth::entity_manager::recipe_step

namespace std
{

template <>
struct is_error_code_enum<wholth::entity_manager::recipe_step::Code> : true_type
{
};

} // namespace std

#endif // WHOLTH_EM_RECIPE_STEP_H_
