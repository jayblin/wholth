#include "wholth/c/internal.hpp"
#include "wholth/c/buffer.h"

auto wholth::c::internal::global_context() -> wholth::Context&
{
    static wholth::Context g_context{};

    return g_context;
}

auto wholth::c::internal::ec_to_error(
    std::error_code ec,
    wholth_Buffer* const buffer) -> wholth_Error
{
    auto msg = ec.message();
    wholth_buffer_move_data_to(buffer, &msg);
    return {
        .code = ec.value(),
        // .message = push_error(ec.message()),
        .message = wholth_buffer_view(buffer),
    };
}

auto wholth::c::internal::str_to_error(
    std::string str,
    wholth_Buffer* const buffer) -> wholth_Error
{
    wholth_buffer_move_data_to(buffer, &str);
    return {
        .code = 0,
        // .message = push_error(std::move(str)),
        .message = wholth_buffer_view(buffer),
    };
}

auto wholth::c::internal::bad_buffer_error() -> wholth_Error
{
    constexpr std::string_view msg = "BUFFER_BAD";
    return {
        .code = 1,
        .message = {
            .data = msg.data(),
            .size = msg.size(),
        }};
}
