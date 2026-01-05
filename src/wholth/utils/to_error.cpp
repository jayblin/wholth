#include "wholth/utils/to_error.hpp"
#include "wholth/c/error.h"
#include "wholth/error.hpp"
#include <system_error>

wholth_Error wholth::utils::from_error_code(const std::error_code& ec)
{
    const auto msg = wholth::error::message(ec.value());
    // const auto& msg = ec.message();
    return {
        .code = static_cast<wholth_ErrorCode>(ec.value()),
        .message = {.data = msg.data(), .size = msg.size()}};
}
