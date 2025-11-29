#include "wholth/c/pages/food.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/forward.h"
#include "wholth/c/internal.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/pages/food.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/utils.hpp"
#include <limits>
#include <memory>
#include <mutex>
#include <type_traits>
#include <variant>

using wholth::pages::internal::PageType;

static_assert(wholth::entity::is_food<wholth_Food>);

static const auto& g_context = wholth::c::internal::global_context();

static bool check_ptr(const wholth_Page* const p)
{
    return nullptr != p && p->data.index() == PageType::FOOD;
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
    ;
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

extern "C" const wholth_FoodArray wholth_pages_food_array(
    const wholth_Page* const p)
{
    if (!check_ptr(p) || 0 == p->pagination.span_size())
    {
        return {nullptr, 0};
    }

    const auto& vector = std::get<PageType::FOOD>(p->data)
                             .container.swappable_buffer_views.view_current()
                             .view;

    assertm(
        vector.size() >= p->pagination.span_size(),
        "You done goofed here wholth_pages_food() [1]!");

    return {vector.data(), p->pagination.span_size()};
}

extern "C" const wholth_Food* wholth_pages_food_array_at(
    const wholth_Page* const p,
    unsigned long long idx)
{
    if (!check_ptr(p) || idx >= p->pagination.span_size())
    {
        return nullptr;
    }

    return &std::get<PageType::FOOD>(p->data)
                .container.swappable_buffer_views.view_current()
                .view[idx];
}

// std::mutex g_wholth_pages_food_mutex;
// std::atomic<uint8_t> g_cur_page_idx;
extern "C" wholth_Page* wholth_pages_food(
    uint8_t prefered_slot,
    uint64_t per_page)
{
    // std::lock_guard<std::mutex> lock(g_wholth_pages_food_mutex);

    static std::unique_ptr g_ptrs = std::make_unique<
        std::array<wholth_Page, std::numeric_limits<uint8_t>::max()>>();

    assert(
        "wholth_pages_food PRECONDITION 1" &&
        prefered_slot < g_ptrs.get()->size());

    // if (wholth::pages::internal::PageType::FOOD !=
    //     (*g_ptrs.get())[prefered_slot].data.)
    auto& page = (*g_ptrs.get())[prefered_slot];
    if (!std::holds_alternative<wholth::pages::Food>(page.data))
    {
        page = wholth_Page{
            per_page,
            wholth::pages::Food{
                .slot = prefered_slot, .query = {}, .container = {per_page}}};
    }
    return &page;
    //
    // for (size_t i = 0; i < g_ptrs.size(); i++)
    // {
    //     if (wholth::pages::internal::PageType::FOOD !=
    //     g_ptrs[i].data.index())
    //     {
    //         break;
    //     }
    //
    //     auto& data =
    //         std::get<wholth::pages::internal::PageType::FOOD>(g_ptrs[i].data);
    //     if (data.slot >= 0 && prefered_slot == data.slot)
    //     {
    //         return &g_ptrs[i];
    //     }
    // }
    //
    // // todo think on how to test slot intialization
    // wholth_Page page{per_page, wholth::pages::Food{.slot = prefered_slot}};
    //
    // fmt::print("slot: {}\n", prefered_slot);
    //
    // assert(
    //     "wholth_pages_food() done goofed 1" &&
    //     g_cur_page_idx < (g_ptrs.size() - 1));
    //
    // g_cur_page_idx++;
    // g_ptrs[g_cur_page_idx - 1] = std::move(page);
    //
    // return &g_ptrs[g_cur_page_idx - 1];
    // // g_ptrs.emplace_back(std::move(page));
    //
    // // return &g_ptrs.back();
}
