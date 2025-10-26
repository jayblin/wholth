#ifndef WHOLTH_C_INTERNAL_H_
#define WHOLTH_C_INTERNAL_H_

#include "wholth/buffer_view.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/error.h"
#include "wholth/context.hpp"
#include <system_error>
#include <vector>

namespace wholth::c::internal
{

auto global_context() -> wholth::Context&;
auto ec_to_error(std::error_code, wholth_Buffer* const) -> wholth_Error;
auto str_to_error(std::string, wholth_Buffer* const) -> wholth_Error;
auto bad_buffer_error() -> wholth_Error;

} // namespace wholth::c::internal

namespace wholth::c::pages::internal
{

template <typename T>
auto move_error_code_to_buffer_view(
    const std::error_code& ec,
    wholth::BufferView<std::vector<T>>& buffer_view,
    const T& copyable_element) -> wholth_Error
{
    for (auto& ent : buffer_view.view)
    {
        ent = copyable_element;
    }

    buffer_view.buffer = ec.message();

    return {
        .code = ec.value(),
        .message = {
            .data = buffer_view.buffer.data(),
            .size = buffer_view.buffer.size()}};
}

} // namespace wholth::c::pages::internal

#endif // WHOLTH_C_INTERNAL_H_
