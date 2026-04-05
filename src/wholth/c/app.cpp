#include "wholth/c/app.h"
#include "wholth/app.hpp"
#include "wholth/c/error.h"
#include "wholth/c/internal.hpp"
#include "wholth/context.hpp"
#include <exception>
#include <string_view>
#include <type_traits>
#include <utility>
#include <cassert>

static auto& g_context = wholth::c::internal::global_context();

extern "C" wholth_Error wholth_app_setup(const wholth_AppSetupArgs* const args)
{
    try
    {
        wholth::app::setup(std::string_view{args->db_path}, g_context);
    }
    catch (const std::exception& excp)
    {
        g_context.exception_message = std::move(excp.what());
        auto buffer = wholth_buffer_ring_pool_element();
        return wholth::c::internal::str_to_error(excp.what(), buffer);
    }

    return wholth_Error_OK;
}

extern "C" void wholth_app_password_encryption_secret(wholth_StringView secret)
{
    if (nullptr != secret.data && secret.size > 0)
    {
        g_context.password_encryption_secret = {secret.data, secret.size};
    }
}

extern "C" bool wholth_error_ok(const wholth_Error* err)
{
    // return !(err->message.size > 0);
    return nullptr == err || wholth_Error_OK.code == err->code;
}
