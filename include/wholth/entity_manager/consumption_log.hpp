#ifndef WHOLTH_EM_CONSUMPTION_LOG_H_
#define WHOLTH_EM_CONSUMPTION_LOG_H_

#include "wholth/c/entity_manager/consumption_log.h"
#include <system_error>

namespace wholth::entity_manager::consumption_log
{

using Code = wholth_em_consumption_log_Code;

}; // namespace wholth::entity_manager::consumption_log
   //
std::error_code make_error_code(wholth::entity_manager::consumption_log::Code);

namespace std
{

template <>
struct is_error_code_enum<wholth::entity_manager::consumption_log::Code>
    : true_type
{
};

} // namespace std

#endif // WHOLTH_EM_CONSUMPTION_LOG_H_
