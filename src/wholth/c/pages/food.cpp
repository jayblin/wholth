#include "wholth/c/pages/food.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/error.h"
#include "wholth/c/internal.hpp"
#include "wholth/c/pages/utils.h"
#include "wholth/entity/food.hpp"
#include "wholth/pages/food.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/utils.hpp"
#include <memory>
#include <type_traits>
#include <variant>
#include <cassert>

using wholth::pages::internal::PageType;

static_assert(wholth::entity::is_food<wholth_Food>);

static const auto& g_context = wholth::c::internal::global_context();

static inline bool check_ptr(const wholth_Page* const p)
{
    return nullptr != p && p->data.index() == PageType::FOOD;
}

extern "C" void wholth_pages_food_id(
    wholth_Page* const p,
    wholth_StringView search_id)
{
    if (!check_ptr(p))
    {
        return;
    }

    std::string new_id{};

    if (nullptr != search_id.data && search_id.size > 0)
    {

        new_id = {search_id.data, search_id.size};
    }

    std::get<PageType::FOOD>(p->data).query.id = std::move(new_id);
}

// todo test
extern "C" void wholth_pages_food_title(
    wholth_Page* const p,
    wholth_StringView search_title)
{
    if (!check_ptr(p))
    {
        return;
    }

    std::string new_title{};

    if (nullptr != search_title.data && search_title.size > 0)
    {

        new_title = {search_title.data, search_title.size};
    }

    std::get<PageType::FOOD>(p->data).query.title = std::move(new_title);
}

// todo test
extern "C" void wholth_pages_food_ingredients(
    wholth_Page* const p,
    wholth_StringView search_ingredients)
{
    if (!check_ptr(p))
    {
        return;
    }

    std::string new_ingrs{};

    if (nullptr != search_ingredients.data && search_ingredients.size > 0)
    {

        new_ingrs = {search_ingredients.data, search_ingredients.size};
    }

    std::get<PageType::FOOD>(p->data).query.ingredients = std::move(new_ingrs);
}

// todo remove
extern "C" const wholth_FoodArray wholth_pages_food_array(
    const wholth_Page* const p)
{
    if (!check_ptr(p) || 0 == p->pagination.span_size())
    {
        return {nullptr, 0};
    }

    const auto& vector = std::get<PageType::FOOD>(p->data).container.view;

    assertm(
        vector.size() >= p->pagination.span_size(),
        "You done goofed here wholth_pages_food() [1]!");

    return {vector.data(), p->pagination.span_size()};
}

extern "C" wholth_Error wholth_pages_food(wholth_Page** page, uint64_t per_page)
{
    auto err = wholth_pages_new(page);

    if (!wholth_error_ok(&err))
    {
        return err;
    }

    wholth::pages::Food page_data{.query = {}, .container = {}};
    page_data.container.view.resize(per_page);

    **page = {per_page, page_data};

    return wholth_Error_OK;
}
