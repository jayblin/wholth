#include "wholth/app_c.h"
#include "db/db.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "wholth/app.hpp"
#include "wholth/c/error.h"
#include "wholth/c/forward.h"
#include "wholth/c/internal.hpp"
#include "wholth/context.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <exception>
// #include <gsl/string_span>
#include <string_view>
#include <type_traits>
#include <utility>

using namespace std::chrono_literals;

static constexpr auto default_timeout = 300ms;

static auto& g_context = wholth::c::internal::global_context();

extern "C" wholth_Error wholth_app_setup(const wholth_AppSetupArgs* const args)
{
    try
    {
        // todo check this
        // const auto old_ctx = std::move(g_context);

        // g_context.db_path = ctx.db_path;
        /* g_context.locale_id("1"); */
        // g_context.locale_id = "1";

        wholth::app::setup(std::string_view{args->db_path}, g_context);

        // wholth_app_locale_id(args->locale_id);
    }
    /* catch (const std::system_error& err) */
    catch (const std::exception& excp)
    {
        // std::cout << excp.what() << '\n';
        // todo log exception
        g_context.exception_message = std::move(excp.what());
        auto buffer = wholth_buffer_ring_pool_element();
        return wholth::c::internal::push_and_get(excp.what(), buffer);
        /* return false; */
        /* g_context.exception_message = fmt::format( */
        /*     "Caught an exception: {} [{}] {}\n", */
        /*     err.code().category().name(), */
        /*     err.code().value(), */
        /*     err.what()); */
    }

    return wholth_Error_OK;
    // return {nullptr, 0};
}

// extern "C" wholth_StringView wholth_latest_error_message()
// {
//     return {
//         .data = g_context.exception_message.data(),
//         .size = g_context.exception_message.size()};
// }

extern "C" void eueu(wholth_StringView sv)
{
    // std::cout << wholth::utils::to_string_view(sv) << '\n';
}

extern "C" bool wholth_error_ok(const wholth_Error* err)
{
    // return !(err->message.size > 0);
    return nullptr == err || wholth_Error_OK.code == err->code;
}

// todo test!
extern "C" void wholth_user_locale_id(const wholth_StringView locale_id)
{
    assert(nullptr != locale_id.data);
    assert(0 < locale_id.size);

    const auto id = wholth::utils::to_string_view(locale_id);

    assert(wholth::utils::is_valid_id(id));
    // if (!wholth::utils::is_valid_id(id))
    // {
    //     return;
    // }

    const auto ec = (sqlw::Transaction{&db::connection()})(
        "UPDATE app_info SET value = ?1 WHERE field = 'default_locale_id'",
        std::array<sqlw::Statement::bindable_t, 1>{
            {{id, sqlw::Type::SQL_INT}}});

    assert("BAD locale_id assignment!" && sqlw::status::Condition::OK == ec);
}

/* static_assert(std::is_same_v<std::remove_cvref_t<const std::string_view&>,
 * std::string_view>); */

/* extern "C" wholth_StringView wholth_foods_page_pagination() */
/* { */
/*     const auto pagination = foods_page_model().page.pagination(); */
/*     return {.data = pagination.data(), .size = pagination.size()}; */
/* } */

/* extern "C" int64_t wholth_foods_page_current_page() */
/* { */
/*     return foods_page_model().page.current_page(); */
/* } */

/* extern "C" int64_t wholth_foods_page_max_page() */
/* { */
/*     return foods_page_model().page.max_page(); */
/* } */

/* extern "C" const wholth_FoodsView internal_preview_wholth_foods() */
/* { */
/*     static int idx = 0; */
/*     idx++; */
/*     static wholth_ShortenedFood g_gfw_arr[2]{ */
/*         wholth_ShortenedFood{ */
/*             .id = {"1", 1}, */
/*             .title = {"Tomato", 6}, */
/*             .preparation_time = {"300s", 4}, */
/*             .top_nutrient = {"30Kc", 4}}, */
/*         wholth_ShortenedFood{ */
/*             .id = {"2", 1}, */
/*             .title = {"Gigachado Avocado Pepperino mamma mia pizzaria
 * beladonna guatemala", 66}, */
/*             .preparation_time = {"400s", 4}, */
/*             .top_nutrient = {"40Kc", 4}}, */
/*     }; */
/*     static wholth_ShortenedFood g_gfw_arr_2[2]{ */
/*         wholth_ShortenedFood{ */
/*             .id = {"3", 1}, */
/*             .title = {"Yomato", 6}, */
/*             .preparation_time = {"500s", 4}, */
/*             .top_nutrient = {"33Kc", 4}}, */
/*         wholth_ShortenedFood{ */
/*             .id = {"4", 1}, */
/*             .title = {"Rotato", 6}, */
/*             .preparation_time = {"500s", 4}, */
/*             .top_nutrient = {"44Kc", 4}}, */
/*     }; */
/*     static wholth_FoodsView g_gfw{.data = g_gfw_arr, .size = 2}; */
/*     static wholth_FoodsView g_gfw_2{.data = g_gfw_arr_2, .size = 2}; */

/*     return idx % 2 ? g_gfw : g_gfw_2; */
/* } */
