#include "wholth/app_c.h"
#include "sqlw/forward.hpp"
#include "wholth/app.hpp"
#include "wholth/context.hpp"
#include "wholth/controller/abstract_page.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/model/foods_page.hpp"
#include <exception>
#include <gsl/string_span>
#include <type_traits>
#include <utility>

using namespace std::chrono_literals;

static constexpr auto default_timeout = 300ms;
static wholth::Context g_context{};
static wholth::model::FoodsContainer<wholth_Food, 20> g_foods_container;
static wholth::model::NutrientsContainer<wholth_Nutrient, 20>
    g_nutrients_container;

static auto& foods_page_model()
{
    static wholth::model::FoodsPage g_foods_page_model{
        g_context, g_foods_container.size};
    return g_foods_page_model;
}

static auto& food_nutrients_page_model()
{
    static wholth::model::NutrientsPage g_food_nutrients_page_model{
        g_context, g_nutrients_container.size};
    return g_food_nutrients_page_model;
}

extern "C" struct wholth_StringView wholth_app_setup(
    const struct WholthContext ctx)
{
    try
    {
        // todo check this
        const auto old_ctx = std::move(g_context);

        g_context.db_path = ctx.db_path;
        /* g_context.locale_id("1"); */
        g_context.locale_id = "1";

        wholth::app::setup(g_context);
    }
    /* catch (const std::system_error& err) */
    catch (const std::exception& excp)
    {
        // todo log exception
        std::cout << "HELLOOO!" << "\n";
        g_context.exception_message = std::move(excp.what());
        std::cout << g_context.exception_message << "\n";
        return {
            g_context.exception_message.data(),
            g_context.exception_message.size()};
        /* return false; */
        /* g_context.exception_message = fmt::format( */
        /*     "Caught an exception: {} [{}] {}\n", */
        /*     err.code().category().name(), */
        /*     err.code().value(), */
        /*     err.what()); */
    }
    std::string_view st{"bogus!"};
    return {st.data(), st.size()};

    /* return true; */
    return {nullptr, 0};
}

extern "C" bool wholth_foods_page_advance()
{
    return wholth::controller::advance(foods_page_model());
}

extern "C" bool wholth_foods_page_retreat()
{
    return wholth::controller::retreat(foods_page_model());
}

extern "C" void wholth_foods_page_fetch()
{
    // todo fix warn
    wholth::controller::fill_container_through_model(
        g_foods_container, foods_page_model(), g_context.connection);
}

extern "C" const wholth_FoodArray wholth_foods_page_list()
{
    if (sqlw::status::Condition::OK != g_context.connection.status())
    {
        // todo show error here?
        return {nullptr, 0};
    }

    const auto& vector =
        g_foods_container.swappable_buffer_views.view_current().view;

    return {vector.data(), vector.size()};
}

extern "C" bool wholth_foods_page_is_fetching()
{
    return foods_page_model().is_fetching;
}

extern "C" wholth_Page wholth_foods_page_info()
{
    const auto page = foods_page_model().pagination;

    return {
        .max_page = page.max_page(),
        .cur_page = page.current_page(),
        .pagination = {
            .data = page.pagination().data(),
            .size = page.pagination().size(),
        }};
}

extern "C" wholth_ErrorCode wholth_entity_food_insert(
    struct wholth_StringView title,
    struct wholth_StringView description)
{
    /* assert(g_context.locale_id().size() > 0); */
    assert(g_context.locale_id.size() > 0);

    std::string id;

    /* auto ec = wholth::entity::food::insert( */
    /*     { */
    /*         .title={title.data, title.size}, */
    /*         .description={description.data, description.size} */
    /*     }, */
    /*     g_context.locale_id(), */
    /*     id, */
    /*     g_context.connection */
    /* ); */

    /* if (wholth::status::Condition::OK != ec) { */
    /*     g_context.exception_message = ec.message(); */
    /*     return ec.value(); */
    /* } */

    return WHOLTH_NO_ERROR;
}

extern "C" wholth_ErrorMessage wholth_latest_error_message()
{
    return {
        .data = g_context.exception_message.data(),
        .size = g_context.exception_message.size()};
}

extern "C" void wholth_entity_food_fetch_nutrients(
    const struct wholth_StringView food_id)
{
    auto& model = food_nutrients_page_model();
    // todo precondition
    model.food_id = {food_id.data, food_id.size};
    // todo fix warn
    wholth::controller::fill_container_through_model(
        g_nutrients_container, food_nutrients_page_model(), g_context.connection);
}

extern "C" const wholth_NutrientArray wholth_entity_food_nutrients()
{
    const auto& vector =
        g_nutrients_container.swappable_buffer_views.view_current().view;

    return {vector.data(), vector.size()};
}

void wholth_entity_food_detail(
    struct wholth_Food* food,
    struct wholth_NutrientArray nutrients,
    struct wholth_IngredientArray ingredients,
    struct wholth_RecipeStepArray recipe_steps)
{
}

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
