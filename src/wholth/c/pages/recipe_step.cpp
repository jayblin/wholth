#include "sqlw/statement.hpp"
#include "wholth/c/pages/recipe_step.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/pages/recipe_step.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/length_container.hpp"
#include "wholth/utils/to_string_view.hpp"
#include <memory>

constexpr auto field_count = 3;

auto wholth::pages::hydrate(
    wholth::pages::RecipeStep& data,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void
{
    auto& buffer = data.container.buffer;
    auto& vec = data.container.view;

    assert(vec.size() > index);

    wholth::utils::extract(vec[index].id, lc, buffer);
    wholth::utils::extract(vec[index].time, lc, buffer);
    wholth::utils::extract(vec[index].description, lc, buffer);
}

auto wholth::pages::prepare_recipe_step_stmt(
    sqlw::Statement& stmt,
    const RecipeStepQuery& model,
    const wholth::Pagination& _)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    using wholth::entity::LengthContainer;
    using wholth::utils::ok;

    constexpr std::string_view sql = R"sql(
    WITH the_list as (
        SELECT
            rs.id,
            seconds_to_readable_time(rs.seconds),
            COALESCE(rsl.description, '[N/A]')
        FROM recipe_step rs
        LEFT JOIN recipe_step_localisation rsl
            ON rs.id = rsl.recipe_step_id
                AND rsl.locale_id = (SELECT value FROM app_info WHERE field = 'default_locale_id')
        WHERE rs.recipe_id = ?1
        ORDER BY rs.priority ASC
    )
    SELECT * FROM (VALUES (1, NULL, NULL))
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT 1)
    )sql";

    ok(stmt.prepare(sql)) //
        && ok(stmt.bind(1, model.recipe_id, sqlw::Type::SQL_INT));

    return {LengthContainer{field_count}, stmt.status()};
}

extern "C" wholth_Error wholth_pages_recipe_step(wholth_Page** page)
{
    auto err = wholth_pages_new(page);

    if (!wholth_error_ok(&err))
    {
        return err;
    }

    constexpr auto per_page = 1;
    wholth::pages::RecipeStep page_data{.query = {}, .container = {}};
    page_data.container.view.resize(per_page);

    **page = {per_page, page_data};

    return wholth_Error_OK;
}

static bool check_page(const wholth_Page* const page)
{
    return nullptr != page &&
           wholth::pages::internal::PageType::RECIPE_STEP == page->data.index();
}

extern "C" const wholth_RecipeStep* wholth_pages_recipe_step_first(
    const wholth_Page* const page)
{
    if (!check_page(page))
    {
        return nullptr;
    }

    const auto& vector =
        std::get<wholth::pages::internal::PageType::RECIPE_STEP>(page->data)
            .container.view;

    if (0 == vector[0].id.size)
    {
        return nullptr;
    }

    return &vector[0];
}

extern "C" void wholth_pages_recipe_step_recipe_id(
    wholth_Page* const page,
    wholth_StringView recipe_id)
{
    if (!check_page(page))
    {
        return;
    }

    std::get<wholth::pages::internal::PageType::RECIPE_STEP>(page->data)
        .query.recipe_id = wholth::utils::to_string_view(recipe_id);
}
