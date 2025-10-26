#ifndef WHOLTH_EM_USER_H_
#define WHOLTH_EM_USER_H_

#include "wholth/c/entity_manager/user.h"
#include <system_error>

namespace wholth::entity_manager::user
{

using Code = wholth_em_user_Code;

}; // namespace wholth::entity_manager::user

std::error_code make_error_code(wholth::entity_manager::user::Code);

namespace std
{

template <>
struct is_error_code_enum<wholth::entity_manager::user::Code> : true_type
{
};

} // namespace std

#endif // WHOLTH_EM_USER_H_
