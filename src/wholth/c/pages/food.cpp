#include "wholth/c/pages/food.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/error.h"
#include "wholth/c/internal.hpp"
#include "wholth/c/pages/utils.h"
#include "wholth/entity/food.hpp"
#include "wholth/error.hpp"
#include "wholth/pages/food.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/to_error.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <memory>
#include <type_traits>
#include <variant>
#include <cassert>

using wholth::pages::internal::PageType;
using wholth::utils::from_error_code;
using wholth::utils::is_valid_id;
using wholth::utils::to_string_view;

template <>
struct wholth::error::is_error_code_enum<wholth_pages_food_Code>
    : std::true_type
{
};

static_assert(wholth::entity::is_food<wholth_Food>);

static const auto& g_context = wholth::c::internal::global_context();

static inline bool check_ptr(const wholth_Page* const p)
{
    return nullptr != p && p->data.index() == PageType::FOOD;
}

enum ModelField : int
{
    FOOD_ID,
    TITLE,
    LOCALE_ID,
    INGREDIENTS,
};

static wholth_Error set_model_field(
    wholth_Page* const page,
    ModelField field,
    wholth_StringView value)
{
    if (!check_ptr(page))
    {
        return wholth::utils::from_error_code(FOOD_PAGE_TYPE_MISMATCH);
    }

    auto& query =
        std::get<wholth::pages::internal::PageType::FOOD>(page->data).query;

    std::error_code ec{};
    switch (field)
    {
    case TITLE:
        query.title = to_string_view(value);
        break;
    case LOCALE_ID: {
        const auto id = to_string_view(value);
        if (!is_valid_id(id))
        {
            ec = FOOD_PAGE_BAD_LOCALE_ID;
        }
        else
        {
            query.locale_id = id;
        }
        break;
    }
    case FOOD_ID: {
        const auto id = to_string_view(value);
        if (!is_valid_id(id))
        {
            ec = FOOD_PAGE_BAD_FOOD_ID;
        }
        else
        {
            query.id = id;
        }
        break;
    }
    case INGREDIENTS:
        query.ingredients = to_string_view(value);
        break;
    }

    return ec ? from_error_code(ec) : wholth_Error_OK;
}

extern "C" wholth_Error wholth_pages_food_id(
    wholth_Page* const p,
    wholth_StringView search_id)
{
    return set_model_field(p, ModelField::FOOD_ID, search_id);
}

// todo test
extern "C" wholth_Error wholth_pages_food_title(
    wholth_Page* const p,
    wholth_StringView search_title)
{
    return set_model_field(p, ModelField::TITLE, search_title);
}

// todo test
extern "C" wholth_Error wholth_pages_food_ingredients(
    wholth_Page* const p,
    wholth_StringView search_ingredients)
{
    return set_model_field(p, ModelField::INGREDIENTS, search_ingredients);
}

// todo test
extern "C" wholth_Error wholth_pages_food_locale_id(
    wholth_Page* const p,
    wholth_StringView locale_id)
{
    return set_model_field(p, ModelField::LOCALE_ID, locale_id);
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
