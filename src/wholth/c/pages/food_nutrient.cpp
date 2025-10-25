#include "wholth/entity/nutrient.hpp"
#include "fmt/core.h"
#include "sqlw/statement.hpp"
#include "wholth/c/entity/nutrient.h"
#include "wholth/c/pages/food_nutrient.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/entity_manager/food.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/pages/food_nutrient.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/length_container.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <memory>

static_assert(wholth::entity::is_nutrient<wholth_Nutrient>);
static_assert(wholth::entity::count_fields<wholth_Nutrient>() == 5);

// template <wholth::entity::is_nutrient T>
auto wholth::pages::hydrate( // const std::string& buffer,
    FoodNutrient& data,
    size_t index,
    wholth::entity::LengthContainer& lc)
    // -> T
    -> void
{
    auto& buffer = data.container.swappable_buffer_views.next().buffer;
    auto& vec = data.container.swappable_buffer_views.next().view;

    assert(vec.size() > index);

    wholth::utils::extract(vec[index].id, lc, buffer);
    wholth::utils::extract(vec[index].title, lc, buffer);
    wholth::utils::extract(vec[index].value, lc, buffer);
    wholth::utils::extract(vec[index].unit, lc, buffer);
    wholth::utils::extract(vec[index].position, lc, buffer);
}

auto wholth::pages::prepare_food_nutrient_stmt(
    sqlw::Statement& stmt,
    const FoodNutrientQuery& model,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    using wholth::entity::LengthContainer;
    using wholth::utils::ok;

    if (!utils::is_valid_id(model.food_id))
    {
        return {
            LengthContainer{0}, entity_manager::food::Code::FOOD_INVALID_ID};
    }

    constexpr std::string_view sql_tpl = R"sql(
    WITH the_list as (
        SELECT
            n.id,
            COALESCE(nl.title, '[N/A]') AS title,
            IIF(
                fn.value < 0.00000001,
                ROUND(fn.value * 100000000, 1) || 'e-8',
                IIF(
                    fn.value < 0.0000001,
                    ROUND(fn.value * 10000000, 1) || 'e-7',
                    IIF(
                        fn.value < 0.000001,
                        ROUND(fn.value * 1000000, 1) || 'e-6',
                        IIF(
                            fn.value < 0.00001,
                            ROUND(fn.value * 100000, 1) || 'e-5',
                            IIF(
                                fn.value < 0.0001,
                                ROUND(fn.value * 10000, 1) || 'e-4',
                                IIF(
                                    fn.value < 0.001,
                                    ROUND(fn.value * 1000, 1) || 'e-3',
                                    IIF(
                                        fn.value < 0.01,
                                        ROUND(fn.value * 100, 1) || 'e-2',
                                        IIF(
                                            fn.value < 0.1,
                                            ROUND(fn.value * 10, 1) || 'e-1',
                                            IIF(
                                                fn.value < 10,
                                                ROUND(fn.value, 2),
                                                ROUND(fn.value, 1)
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            ) AS value,
            n.unit,
            n.position
        FROM nutrient n
        INNER JOIN food_nutrient fn
            ON fn.nutrient_id = n.id AND fn.food_id = ?1
        LEFT JOIN nutrient_localisation nl
            ON nl.nutrient_id = n.id
                AND nl.locale_id = (SELECT value FROM app_info WHERE field = 'default_locale_id')
        WHERE
            1=1
            {0}
        ORDER BY n.position ASC, nl.title ASC
    )
    SELECT COUNT(the_list.id), NULL, NULL, NULL, NULL FROM the_list
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT ?2 OFFSET ?3)
    )sql";

    const std::string sql = fmt::format(
        sql_tpl, model.title.size() > 0 ? "AND nl.title LIKE ?4" : "");

    ok(stmt.prepare(sql))                                            //
        && ok(stmt.bind(1, model.food_id, sqlw::Type::SQL_INT))      //
        && ok(stmt.bind(2, static_cast<int>(pagination.per_page()))) //
        && ok(stmt.bind(
               3,
               static_cast<int>(
                   pagination.per_page() * pagination.current_page())));

    if (ok(stmt) && model.title.size() > 0)
    {
        std::string title_param_value = fmt::format("%{0}%", model.title);
        stmt.bind(4, title_param_value, sqlw::Type::SQL_TEXT);
    }

    return {
        wholth::entity::LengthContainer{
            pagination.per_page() *
            wholth::entity::count_fields<wholth_Nutrient>()},
        stmt.status()};
}

extern "C" wholth_Page* wholth_pages_food_nutrient(
    uint64_t per_page,
    bool reset = false)
{
    static std::unique_ptr<wholth_Page> ptr;
    // static std::unique_ptr<wholth_Page> ptr =
    //     std::make_unique<wholth_Page>(per_page,
    //     wholth::pages::FoodNutrient{});

    if (nullptr == ptr.get() || reset)
    {
        ptr = std::make_unique<wholth_Page>(
            per_page,
            wholth::pages::FoodNutrient{.query = {}, .container = {per_page}});
    }

    return ptr.get();
}

static bool check_page(const wholth_Page* const page)
{
    return nullptr != page &&
           wholth::pages::internal::PageType::FOOD_NUTRIENT ==
               page->data.index();
}

extern "C" const wholth_FoodNutrientArray wholth_pages_food_nutrient_array(
    const wholth_Page* const page)
{
    if (!check_page(page) || page->pagination.span_size() == 0)
    {
        return {nullptr, 0};
    }

    const auto& vector =
        std::get<wholth::pages::internal::PageType::FOOD_NUTRIENT>(page->data)
            .container.swappable_buffer_views.view_current()
            .view;

    assertm(
        vector.size() >= page->pagination.span_size(),
        "You done goofed here wholth_pages_nutrient() [1]!");

    return {vector.data(), page->pagination.span_size()};
}

extern "C" void wholth_pages_food_nutrient_food_id(
    wholth_Page* const page,
    wholth_StringView food_id)
{
    if (!check_page(page))
    {
        return;
    }

    const auto _fi = wholth::utils::to_string_view(food_id);

    if (!wholth::utils::is_valid_id(_fi))
    {
        return;
    }

    std::get<wholth::pages::internal::PageType::FOOD_NUTRIENT>(page->data)
        .query.food_id = _fi;
}

extern "C" void wholth_pages_food_nutrient_title(
    wholth_Page* const page,
    wholth_StringView title)
{
    if (!check_page(page))
    {
        return;
    }

    std::get<wholth::pages::internal::PageType::FOOD_NUTRIENT>(page->data)
        .query.title = wholth::utils::to_string_view(title);
}
