#include "wholth/c/pages/ingredient.h"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/entity/length_container.hpp"
#include "wholth/entity_manager/food.hpp"
#include "wholth/pages/ingredient.hpp"
#include "wholth/pages/internal.hpp"
#include "wholth/utils.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/length_container.hpp"
#include <cstddef>
#include <memory>

constexpr auto field_count = 5;
using wholth::pages::internal::PageType;

auto wholth::pages::hydrate(
    Ingredient& data,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void
{
    auto& buffer = data.container.buffer;
    auto& vec = data.container.view;

    assert(vec.size() > index);

    wholth::utils::extract(vec[index].id, lc, buffer);
    wholth::utils::extract(vec[index].food_id, lc, buffer);
    wholth::utils::extract(vec[index].food_title, lc, buffer);
    wholth::utils::extract(vec[index].canonical_mass_g, lc, buffer);
    wholth::utils::extract(vec[index].ingredient_count, lc, buffer);
}

auto wholth::pages::prepare_ingredient_stmt(
    sqlw::Statement& stmt,
    const IngredientQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    using wholth::entity::LengthContainer;
    using wholth::utils::ok;

    if (!wholth::utils::is_valid_id(query.food_id))
    {
        return {
            LengthContainer{0},
            wholth::entity_manager::food::Code::FOOD_INVALID_ID};
    }

    constexpr std::string_view sql = R"sql(
    WITH the_list as (
        SELECT
            rsf.id AS id,
            f.id AS food_id,
            COALESCE(fl.title, '[N/A]') AS food_title,
            rsf.canonical_mass AS canonical_mass_g,
            (
                SELECT
                    COUNT(rsf2.food_id)
                FROM recipe_step rs2
                LEFT JOIN recipe_step_food rsf2
                    ON rsf2.recipe_step_id = rs2.id
                WHERE rs2.recipe_id = f.id
            ) AS ingredient_count
        FROM recipe_step rs
        INNER JOIN recipe_step_food rsf
            ON rsf.recipe_step_id = rs.id
        LEFT JOIN food f
            ON f.id = rsf.food_id
        LEFT JOIN food_localisation fl
            ON fl.food_id = f.id
                AND fl.locale_id = (SELECT value FROM app_info WHERE field = 'default_locale_id')
        WHERE
            rs.recipe_id = ?1
            AND (fl.title IS NULL OR fl.title LIKE ?4)
        ORDER BY f.id ASC
    )
    SELECT COUNT(the_list.id), NULL, NULL, NULL, NULL FROM the_list
    UNION ALL
    SELECT * FROM (SELECT * FROM the_list LIMIT ?2 OFFSET ?3)
    )sql";

    const std::string title = fmt::format("%{}%", query.title);
    ok(stmt.prepare(sql)) &&
        ok(stmt.bind(1, query.food_id, sqlw::Type::SQL_INT)) &&
        ok(stmt.bind(2, static_cast<int>(pagination.per_page()))) &&
        ok(stmt.bind(
            3,
            static_cast<int>(
                pagination.per_page() * pagination.current_page()))) &&
        ok(stmt.bind(4, title, sqlw::Type::SQL_TEXT));
    // todo check int static_cast

    return {
        LengthContainer{pagination.per_page() * field_count}, stmt.status()};
}

static bool check_page(const wholth_Page* const page)
{
    return nullptr != page && PageType::INGREDIENT == page->data.index();
}

extern "C" wholth_Error wholth_pages_ingredient(
    wholth_Page** page,
    uint64_t per_page)
{
    auto err = wholth_pages_new(page);

    if (!wholth_error_ok(&err))
    {
        return err;
    }

    wholth::pages::Ingredient page_data{.query = {}, .container = {}};
    page_data.container.view.resize(per_page);

    **page = {per_page, page_data};

    return wholth_Error_OK;
}

extern "C" const wholth_IngredientArray wholth_pages_ingredient_array(
    const wholth_Page* const page)
{
    if (!check_page(page) || page->pagination.span_size() == 0)
    {
        return {nullptr, 0};
    }

    const auto& vector =
        std::get<PageType::INGREDIENT>(page->data).container.view;

    assertm(
        vector.size() >= page->pagination.span_size(),
        "You done goofed here wholth_pages_ingredient_array() [1]!");

    return {vector.data(), page->pagination.span_size()};
}

extern "C" void wholth_pages_ingredient_title(
    wholth_Page* const page,
    wholth_StringView title)
{
    if (check_page(page))
    {
        std::get<PageType::INGREDIENT>(page->data).query.title = {
            title.data, title.size};
    }
}

extern "C" void wholth_pages_ingredient_food_id(
    wholth_Page* const page,
    wholth_StringView food_id)
{
    if (check_page(page))
    {
        std::get<PageType::INGREDIENT>(page->data).query.food_id = {
            food_id.data, food_id.size};
    }
}
