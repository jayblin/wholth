#include "fmt/core.h"
#include "sqlw/statement.hpp"
#include "wholth/c/pages/nutrient.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/model/abstract_page.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/pages/nutrient.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/length_container.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <memory>

auto wholth::pages::hydrate(
    wholth::pages::Nutrient& data,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void
{
    auto& buffer = data.container.swappable_buffer_views.next().buffer;
    auto& vec = data.container.swappable_buffer_views.next().view;

    assert(vec.size() > index);

    wholth::utils::extract(vec[index].id, lc, buffer);
    wholth::utils::extract(vec[index].title, lc, buffer);
    wholth::utils::extract(vec[index].unit, lc, buffer);
    wholth::utils::extract(vec[index].position, lc, buffer);
}

auto wholth::pages::prepare_nutrient_stmt(
    sqlw::Statement& stmt,
    const NutrientQuery& model,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    using wholth::entity::LengthContainer;
    using wholth::utils::ok;

    constexpr std::string_view sql_tpl = R"sql(
    WITH the_list as (
        SELECT
            n.id,
            COALESCE(nl.title, '[N/A]'),
            n.unit,
            n.position
        FROM nutrient n
        LEFT JOIN nutrient_localisation nl
            ON nl.nutrient_id = n.id
                AND nl.locale_id = (SELECT value FROM app_info WHERE field = 'default_locale_id')
        WHERE {0}
        ORDER BY n.position ASC
    )
    SELECT COUNT(the_list.id), NULL, NULL, NULL FROM the_list
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT ?1 OFFSET ?2)
    )sql";

    const std::string sql =
        fmt::format(sql_tpl, model.title.empty() ? "1=1" : "nl.title LIKE ?3");
    const std::string title_param =
        model.title.empty() ? "1=1" : fmt::format("%{}%", model.title);

    ok(stmt.prepare(sql))                                            //
        && ok(stmt.bind(1, static_cast<int>(pagination.per_page()))) //
        && ok(stmt.bind(
               2,
               static_cast<int>(
                   pagination.per_page() * pagination.current_page())));

    if (ok(stmt) && !model.title.empty())
    {
        std::string title_param_value = fmt::format("%{0}%", model.title);
        stmt.bind(3, title_param_value, sqlw::Type::SQL_TEXT);
    }

    return {LengthContainer{pagination.per_page() * 4}, stmt.status()};
}

extern "C" wholth_Page* wholth_pages_nutrient(
    uint64_t per_page,
    bool reset = false)
{
    static std::unique_ptr<wholth_Page> ptr;

    if (nullptr == ptr.get() || reset)
    {
        ptr = std::make_unique<wholth_Page>(
            per_page,
            wholth::pages::Nutrient{.query = {}, .container = {per_page}});
    }

    return ptr.get();
}

static bool check_page(const wholth_Page* const page)
{
    return nullptr != page &&
           wholth::pages::internal::PageType::NUTRIENT == page->data.index();
}

extern "C" const wholth_NutrientArray wholth_pages_nutrient_array(
    const wholth_Page* const page)
{
    if (!check_page(page) || page->pagination.span_size() == 0)
    {
        return {nullptr, 0};
    }

    const auto& vector =
        std::get<wholth::pages::internal::PageType::NUTRIENT>(page->data)
            .container.swappable_buffer_views.view_current()
            .view;

    assertm(
        vector.size() >= page->pagination.span_size(),
        "You done goofed here wholth_pages_nutrient() [1]!");

    return {vector.data(), page->pagination.span_size()};
}

extern "C" void wholth_pages_nutrient_title(
    wholth_Page* const page,
    wholth_StringView title)
{
    if (!check_page(page))
    {
        return;
    }

    std::get<wholth::pages::internal::PageType::NUTRIENT>(page->data)
        .query.title = wholth::utils::to_string_view(title);
}
