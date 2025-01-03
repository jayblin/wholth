#ifndef UI_COMPONENTS_FOOD_LIST_H_
#define UI_COMPONENTS_FOOD_LIST_H_

#include "imgui.h"
#include "imgui_internal.h"
#include "ui/components/search_bap.hpp"
#include "ui/components/timer.hpp"
#include "ui/style.hpp"
#include "wholth/context.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list/food.hpp"
#include <cstdint>
#include <span>
#include <thread>

namespace ui::components
{
    template <size_t Size>
    class FoodList
    {
    public:
        /* FoodList( */
        /*     wholth::Context& ctx */
        /*     /1* wholth::list::food::Query query, *1/ */
        /*     /1* const ui::Style& style, *1/ */
        /*     /1* sqlw::Connection& con *1/ */
        /* ); */

        /* template <typename T> */
        // todo requires 
        void render(
            const std::chrono::duration<double>& delta,
            const ui::Style& m_style,
            std::span<const wholth::entity::shortened::Food> items,
            std::function<void (const wholth::entity::shortened::Food&)> render_item
        );

        /* auto count() const { return m_count; } */
        /* auto max_page() const { return m_max_page; } */
        /* auto pagination() const -> const std::string_view { return m_pagination_str; } */
        /* auto is_fetching() const -> bool { return m_is_fetching_list; } */
        /* /1* auto fetch(wholth::list::food::Query& query) -> void; *1/ */
        /* // todo: think about if passing query by values is ok */
        /* auto fetch(wholth::list::food::Query query) -> void; */
        /* auto advance() -> void; */
        /* auto retreat() -> void; */

    private:
        /* auto update_pagination() -> void; */

        /* wholth::Context& m_ctx; */
        /* sqlw::Connection& m_con; */
        /* const ui::Style& m_style; */

        /* wholth::list::food::Query m_query {}; */

        /* std::array< */
        /*     std::array<wholth::entity::shortened::Food, Size>, */
        /*     2 */
        /* > m_list; */
        /* std::array<std::string, 2> m_list_buffer {}; */
        /* // todo: retype to uint */
        /* std::atomic<int64_t> m_max_page {0}; */
        /* uint64_t m_count {0}; */
        /* std::string m_pagination_str {""}; */
        /* /1* uint64_t m_cur_page {0}; *1/ */
        /* ui::components::Timer m_timer {}; */
        /* std::atomic<int64_t> m_cur_idx {0}; */
        /* std::atomic<bool> m_is_fetching_list {false}; */
        /* std::chrono::milliseconds m_fetch_duration {0ms}; */
        /* std::mutex m_mutex; */

        /* Listener m_listener {}; */
    };
}

/* template<size_t Size> */
/* ui::components::FoodList<Size>::FoodList( */
/*     wholth::Context& ctx */
/* ) */
/*     : m_ctx(ctx) */
/* { */
/*     ctx.locale_id.subscribe([&] (const auto& locale_id) { */
/*         m_query.locale_id = locale_id; */
/*         fetch(m_query); */
/*     }); */
/* } */

/* template<size_t Size> */
/* void ui::components::FoodList<Size>::render( */
/*     const std::chrono::duration<double>& delta, */
/*     /1* T& component *1/ */
/*     std::function<void (const wholth::entity::shortened::Food&)> render_item */
/* ) */
/* { */
/*     m_timer.tick(delta); */

/*     if (!m_is_fetching_list && m_timer.has_expired()) */
/*     { */
/*         fetch(m_query); */
/*     } */

/*     int i = 0; */
/*     for (const auto& item : m_list[m_cur_idx]) */
/*     { */
/*         i++; */

/*         if (item.id.empty()) { */
/*             break; */
/*         } */

/*         ImGui::PushID(i); */

/*         render_item(item); */
/*         /1* component.render(delta, item); *1/ */

/*         ImGui::PopID(); */
/*     } */
/* } */

/* template<size_t Size> */
/* void ui::components::FoodList<Size>::fetch( */
/*     wholth::list::food::Query query */
/*     /1* std::function<void ()> callback *1/ */
/* ) */
/* { */
/*     std::thread th1([this, query]() { // is this capture ok? idk */
/*         std::lock_guard guard {m_mutex}; */

/*         m_is_fetching_list.store(true, std::memory_order_seq_cst); */

/*         int64_t prev_idx = m_cur_idx; */
/*         auto tmp_idx = (m_cur_idx + 1) % m_list.size(); */

/*         // todo: is this thread-safe? */
/*         m_query = std::move(query); */

/*         wholth::list::food::list( */
/*             m_list[tmp_idx], */
/*             m_count, */
/*             m_list_buffer[tmp_idx], */
/*             query, */
/*             /1* m_con *1/ */
/*             m_ctx.connection */
/*         ); */

/*         m_cur_idx.store(tmp_idx, std::memory_order_seq_cst); */
/*         m_is_fetching_list.store(false, std::memory_order_seq_cst); */

/*         // todo: check for thread safety */
/*         update_pagination(); */

/*         for (auto& e : m_list[prev_idx]) { */
/*             e = {}; */
/*         } */
/*     }); */

/*     th1.detach(); */
/* } */

/* template<size_t Size> */
/* void ui::components::FoodList<Size>::update_pagination() */
/* { */
/*     /1* Expects(1 == 1); *1/ */
/*     m_max_page = std::ceil(static_cast<float>(m_count) / static_cast<float>(Size)); */

/*     m_pagination_str = fmt::format( */
/*         "{}/{}", */
/*         // todo: check bounds */
/*         /1* m_cur_page + 1, *1/ */
/*         m_query.page + 1, */
/*         m_max_page.load() */
/*     ); */
/* } */

/* template<size_t Size> */
/* void ui::components::FoodList<Size>::advance() */
/* { */
/*     if (!m_is_fetching_list && (m_query.page + 1) < m_max_page) { */
/*     /1* if (!m_is_fetching_list && (m_cur_page + 1) < m_max_page) { *1/ */
/*         m_query.page++, */
/*         /1* m_cur_page++, *1/ */
/*         update_pagination(); */
/*         m_timer.start(); */
/*     } */
/* } */

/* template<size_t Size> */
/* void ui::components::FoodList<Size>::retreat() */
/* { */
/*     if (!m_is_fetching_list && m_query.page > 0) { */
/*     /1* if (!m_is_fetching_list && m_cur_page > 0) { *1/ */
/*         m_query.page--; */
/*         /1* m_cur_page--; *1/ */
/*         update_pagination(); */
/*         m_timer.start(); */
/*     } */
/* } */

#endif // UI_COMPONENTS_FOOD_LIST_H_
