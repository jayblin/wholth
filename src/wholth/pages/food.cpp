#include "fmt/core.h"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/c/entity/food.h"
#include "wholth/entity/length_container.hpp"
#include "wholth/pages/code.hpp"
#include "wholth/pages/food.hpp"
#include "wholth/pagination.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include "wholth/utils/length_container.hpp"
#include <sstream>
#include <cassert>

static_assert(wholth::entity::is_food<wholth_Food>);
static_assert(wholth::entity::count_fields<wholth_Food>() == 4);

using SC = wholth::pages::Code;

auto wholth::pages::hydrate(
    Food& food,
    size_t index,
    wholth::entity::LengthContainer& lc) -> void
{
    auto& buffer = food.container.buffer;
    auto& vec = food.container.view;

    assert(vec.size() > index);

    wholth::utils::extract(vec[index].id, lc, buffer);
    wholth::utils::extract(vec[index].title, lc, buffer);
    wholth::utils::extract(vec[index].preparation_time, lc, buffer);
    wholth::utils::extract(vec[index].top_nutrient, lc, buffer);
}

// @todo rename `idx`.
static void bind_params(
    sqlw::Statement& stmt,
    /* const whf::Query& q, */
    const wholth::pages::FoodQuery& q,
    size_t idx)
{
    /* using OP = whf::nutrient_filter::Operation; */

    /* for (const whf::nutrient_filter::Entry& entry : q.nutrient_filters) */
    /* { */
    /* 	if (is_empty(entry)) */
    /* 	{ */
    /* 		break; */
    /* 	} */

    /* 	const auto& value = std::get<whf::nutrient_filter::Value::index>(entry);
     */

    /* 	switch (std::get<OP>(entry)) */
    /* 	{ */
    /* 		case OP::EQ: { */
    /* 			stmt.bind( */
    /* 				idx, */
    /* 				std::get<whf::nutrient_filter::NutrientId::index>(entry), */
    /* 				sqlw::Type::SQL_INT */
    /* 			); */
    /* 			idx++; */
    /* 			stmt.bind( */
    /* 				idx, */
    /* 				value, */
    /* 				sqlw::Type::SQL_DOUBLE */
    /* 			); */
    /* 			idx++; */
    /* 			break; */
    /* 		} */
    /* 		case OP::NEQ: { */
    /* 			stmt.bind( */
    /* 				idx, */
    /* 				std::get<whf::nutrient_filter::NutrientId::index>(entry), */
    /* 				sqlw::Type::SQL_INT */
    /* 			); */
    /* 			idx++; */
    /* 			stmt.bind( */
    /* 				idx, */
    /* 				value, */
    /* 				sqlw::Type::SQL_DOUBLE */
    /* 			); */
    /* 			idx++; */
    /* 			break; */
    /* 		} */
    /* 		case OP::BETWEEN: { */
    /* 			std::array<std::string_view, 2> values { */
    /* 				"0", */
    /* 				"0" */
    /* 			}; */

    /* 			for (size_t i = 0; i < value.length(); i++) { */
    /* 				if (',' == value[i] && (value.length() - 1) != i) */
    /* 				{ */
    /* 					values[0] = value.substr(0, i); */
    /* 					values[1] = value.substr(i + 1); */
    /* 					break; */
    /* 				} */
    /* 			} */

    /* 			stmt.bind( */
    /* 				idx, */
    /* 				std::get<whf::nutrient_filter::NutrientId::index>(entry), */
    /* 				sqlw::Type::SQL_INT */
    /* 			); */
    /* /1* fmt::print("{} and {}\n", idx,
     * std::get<whf::nutrient_filter::NutrientId::index>(entry)); *1/ */
    /* 			idx++; */
    /* 			stmt.bind( */
    /* 				idx, */
    /* 				values[0], */
    /* 				sqlw::Type::SQL_DOUBLE */
    /* 			); */
    /* /1* fmt::print("{} and {}\n", idx, values[0]); *1/ */
    /* 			idx++; */
    /* 			stmt.bind( */
    /* 				idx, */
    /* 				values[1], */
    /* 				sqlw::Type::SQL_DOUBLE */
    /* 			); */
    /* /1* fmt::print("{} and {}\n", idx, values[1]); *1/ */
    /* 			idx++; */
    /* 			break; */
    /* 		} */
    /* 	} */
    /* } */

    if (q.title.size() > 0)
    {
        std::string _title = fmt::format("%{0}%", q.title);

        stmt.bind(idx, _title, sqlw::Type::SQL_TEXT);
        idx++;
    }

    if (q.id.size() > 0)
    {
        stmt.bind(idx, q.id, sqlw::Type::SQL_INT);
        idx++;
    }
}

struct IngredientData
{
    uint64_t parameter_count;
    /**
     * A string like "%protein%%calori%"
     */
    std::string parameter_buffer;
    /**
     * Number of chars for each parameter om...
     */
    std::array<uint64_t, 32> parameter_lengths;
    /**
     * Clauses for WHERE statement to filter ingredients.
     */
    std::string where;
};

auto list_foods_prepare_stmt(
    sqlw::Statement& stmt,
    std::string_view sql,
    const wholth::pages::FoodQuery& q,
    const IngredientData& ingredient_data) -> void
{
    /* const auto& [ingredient_param_count, ingredient_where, ingredient_tokens,
     * ingredient_tokens_lengths] = ingredient_data; */
    /* size_t param_idx = ingredient_data.parameter_count; */

    stmt.prepare(sql);

    size_t param_idx = 1;

    size_t ing_start = 0;
    for (size_t i = 0; i < ingredient_data.parameter_lengths.size(); i++)
    {
        if (0 == ingredient_data.parameter_lengths[i])
        {
            break;
        }

        stmt.bind(
            param_idx,
            std::string_view{
                ingredient_data.parameter_buffer.data() + ing_start,
                ingredient_data.parameter_lengths[i]},
            sqlw::Type::SQL_TEXT);

        ing_start += ingredient_data.parameter_lengths[i];
        param_idx++;
    }

    bind_params(stmt, q, param_idx);
}

std::string create_where(
    const wholth::pages::FoodQuery& q,
    size_t sql_param_idx)
{
    /* using OP = whf::nutrient_filter::Operation; */

    std::stringstream tpl_stream;
    /* size_t sql_param_idx = 2; */

    /* for (size_t i = 0; i < q.nutrient_filters.size(); i++) */
    /* { */
    /* 	const auto& entry = q.nutrient_filters[i]; */

    /* 	if (is_empty(entry)) */
    /* 	{ */
    /* 		break; */
    /* 	} */

    /* 	switch (std::get<OP>(entry)) */
    /* 	{ */
    /* 		case OP::EQ: { */
    /* 			auto idx1 = sql_param_idx; */
    /* 			sql_param_idx++; */
    /* 			auto idx2 = sql_param_idx; */
    /* 			sql_param_idx++; */
    /* 			tpl_stream << fmt::format( */
    /* 				"INNER JOIN food_nutrient AS fn{} " */
    /* 				" ON fn{}.food_id = rt.recipe_id " */
    /* 				" AND fn{}.nutrient_id = ?{} " */
    /* 				" AND (fn{}.value <= (?{} + 0.001) AND fn{}.value >= (?{} -
     * 0.001)) ", */
    /* 				i, */
    /* 				i, */
    /* 				i, */
    /* 				idx1, */
    /* 				i, */
    /* 				idx2, */
    /* 				i, */
    /* 				idx2 */
    /* 			); */
    /* 			break; */
    /* 		} */
    /* 		case OP::NEQ: { */
    /* 			auto idx1 = sql_param_idx; */
    /* 			sql_param_idx++; */
    /* 			auto idx2 = sql_param_idx; */
    /* 			sql_param_idx++; */
    /* 			tpl_stream << fmt::format( */
    /* 				"INNER JOIN food_nutrient AS fn{} " */
    /* 				" ON fn{}.food_id = rt.recipe_id " */
    /* 				" AND fn{}.nutrient_id = ?{} " */
    /* 				" AND (fn{}.value < (?{} - 0.001) or fn{}.value > (?{} +
     * 0.001)) ", */
    /* 				i, */
    /* 				i, */
    /* 				i, */
    /* 				idx1, */
    /* 				i, */
    /* 				idx2, */
    /* 				i, */
    /* 				idx2 */
    /* 			); */
    /* 			break; */
    /* 		} */
    /* 		case OP::BETWEEN: { */
    /* 			auto idx1 = sql_param_idx; */
    /* 			sql_param_idx++; */
    /* 			auto idx2 = sql_param_idx; */
    /* 			sql_param_idx++; */
    /* 			auto idx3 = sql_param_idx; */
    /* 			sql_param_idx++; */

    /* 			tpl_stream << fmt::format( */
    /* 				" INNER JOIN food_nutrient AS fn{} " */
    /* 				" ON fn{}.food_id = rt.recipe_id " */
    /* 				" AND fn{}.nutrient_id = ?{} " */
    /* 				" AND (fn{}.value >= ?{} AND fn{}.value <= ?{}) ", */
    /* 				i, */
    /* 				i, */
    /* 				i, */
    /* 				idx1, */
    /* 				i, */
    /* 				idx2, */
    /* 				i, */
    /* 				idx3 */
    /* 			); */
    /* 			break; */
    /* 		} */
    /* 	} */
    /* } */
    tpl_stream << " WHERE 1=1 ";

    if (q.title.size() > 0)
    {
        tpl_stream << " AND rt.recipe_title LIKE ?" << sql_param_idx;

        sql_param_idx++;
    }

    if (q.id.size() > 0)
    {
        tpl_stream << " AND rt.recipe_id = ?" << sql_param_idx;

        sql_param_idx++;
    }

    return tpl_stream.str();
}

static auto prepare_ingredient_data(
    /* const whf::Query& q */
    const wholth::pages::FoodQuery& q) -> IngredientData
{
    std::stringstream where_stream;
    std::stringstream tokens_stream;
    size_t start = 0;
    std::array<uint64_t, 32> lengths;
    std::fill(lengths.begin(), lengths.end(), 0);

    uint64_t param_idx = 0;
    for (size_t i = 0; i < q.ingredients.size() && i < lengths.size(); i++)
    {
        const char ch = q.ingredients[i];
        size_t j = i + 1;
        std::string_view sub;

        // Extract the word excluding:
        // - ',';
        // - all spaces before the word.
        if (ch == ',')
        {
            if (start == 0)
            {
                sub = q.ingredients.substr(start, i - start);
            }
            else
            {
                auto first_letter_idx =
                    q.ingredients.find_first_not_of(' ', start);
                sub = q.ingredients.substr(
                    first_letter_idx, i - first_letter_idx);
            }
            start = i + 1;
        }
        else if (q.ingredients.size() == j)
        {
            auto first_letter_idx = q.ingredients.find_first_not_of(' ', start);
            sub = q.ingredients.substr(first_letter_idx, j - first_letter_idx);
        }

        if (sub.size() > 2)
        {
            lengths[param_idx] = sub.size() + 2;
            param_idx++;
            where_stream << "fl.title LIKE ?" << param_idx << " OR ";
            tokens_stream << '%' << sub << '%';
        }
    }

    std::string where = where_stream.str();

    if (where.size() > 4)
    {
        where = where.substr(0, where.size() - 4);
    }
    else
    {
        where = "1";
    }

    return {
        .parameter_count = param_idx,
        .parameter_buffer = tokens_stream.str(),
        .parameter_lengths = lengths,
        .where = where,
    };
}

auto wholth::pages::prepare_food_stmt(
    sqlw::Statement& stmt,
    const wholth::pages::FoodQuery& query,
    const wholth::Pagination& pagination)
    -> std::tuple<wholth::entity::LengthContainer, std::error_code>
{
    // todo check query

    const auto ingredient_data = prepare_ingredient_data(query);

    // top_nutrient scientific notaion
                // COALESCE(
                //     IIF (
                //         mvfn.value < 0.000001,
                //         ROUND(mvfn.value * 1000000, 1) || 'e-6',
                //         IIF(
                //             mvfn.value < 0.00001,
                //             ROUND(mvfn.value * 100000, 1) || 'e-5',
                //             IIF(
                //                 mvfn.value < 0.0001,
                //                 ROUND(mvfn.value * 10000, 1) || 'e-4',
                //                 IIF(
                //                     mvfn.value < 0.001,
                //                     ROUND(mvfn.value * 1000, 1) || 'e-3',
                //                     IIF(
                //                         mvfn.value < 0.01,
                //                         ROUND(mvfn.value * 100, 1) || 'e-2',
                //                         IIF(
                //                             mvfn.value < 0.1,
                //                             ROUND(mvfn.value * 10, 1) || 'e-1',
                //                             IIF(
                //                                 mvfn.value < 10,
                //                                 ROUND(mvfn.value, 2),
                //                                 ROUND(mvfn.value, 1)
                //                             )
                //                         )
                //                     )
                //                 )
                //             )
                //         )
                //     ),
                //     '[N/A]'
                // )
    constexpr std::string_view sql = R"sql(
        WITH RECURSIVE
        recipe_tree(
            lvl,
            recipe_id,
            recipe_title,
            recipe_ingredient_count,
            ingredient_id,
            step_seconds
        ) AS (
            SELECT
                1,
                f.id,
                fl.title,
                0,
                NULL,
                rs.seconds
            FROM food f
            LEFT JOIN recipe_info ri
                ON ri.recipe_id = f.id
            LEFT JOIN recipe_step rs
                ON rs.recipe_id = f.id
            LEFT JOIN food_localisation fl
                 ON fl.food_id = f.id
                    AND fl.locale_id = {4} 
            WHERE {0}
            UNION
            SELECT
                rt.lvl + 1,
                node.recipe_id,
                fl.title,
                node.recipe_ingredient_count,
                node.ingredient_id,
                node.step_seconds
            FROM recipe_tree rt
            INNER JOIN recipe_tree_node node
                ON node.ingredient_id = rt.recipe_id
            LEFT JOIN food_localisation fl
                ON fl.food_id = node.recipe_id
                    AND fl.locale_id = {4} 
            ORDER BY 1 DESC
        ),
        top_nutrient AS NOT MATERIALIZED (
            SELECT
                mvfn.value
                || ' '
                || COALESCE(mvn.unit, '') AS value,
                mvfn.food_id AS recipe_id,
                mvn.position
                -- ROW_NUMBER() OVER (
                --     PARTITION BY rt.recipe_id
                --     ORDER BY mvn.postion ASC
                -- ) AS row_num
            FROM food_nutrient mvfn
            LEFT JOIN nutrient mvn
                ON mvn.id = mvfn.nutrient_id
            ORDER BY mvn.position ASC
        ),
        the_list AS (
            SELECT
                rt.recipe_id AS id,
                COALESCE(rt.recipe_title, '[N/A]') AS title,
                COALESCE(
                    seconds_to_readable_time(rt.step_seconds),
                    '[N/A]'
                ) AS time,
                COALESCE(tp.value, '[N/A]') AS top_nutrient
            FROM recipe_tree rt
            LEFT JOIN top_nutrient tp
                -- todo choose 10 or 11 or 12 or something with min position
                ON tp.recipe_id = rt.recipe_id AND tp.position = 10
            {1}
            GROUP BY rt.recipe_id
            ORDER BY rt.lvl DESC, rt.recipe_id ASC, rt.recipe_title ASC
        )
        SELECT COUNT(the_list.id), NULL, NULL, NULL FROM the_list
        UNION ALL
        SELECT * FROM (SELECT * FROM the_list LIMIT {2} OFFSET {3})
    )sql";

    const auto where = create_where(query, ingredient_data.parameter_count + 1);

    // todo this is bad. validate locale_id and bind it.
    const std::string_view locale_id_query =
        query.locale_id.empty() && utils::is_valid_id(query.locale_id)
            ? std::string_view{query.locale_id.data(), query.locale_id.size()}
            : "(SELECT value FROM app_info WHERE field = 'default_locale_id')";

    list_foods_prepare_stmt(
        stmt,
        fmt::format(
            sql,
            ingredient_data.where,                             // 0
            where,                                             // 1
            pagination.per_page(),                             // 2
            pagination.per_page() * pagination.current_page(), // 3
            locale_id_query                                    // 4
            ),
        query,
        ingredient_data);

    return {
        wholth::entity::LengthContainer{
            pagination.per_page() *
            wholth::entity::count_fields<wholth_Food>()},
        stmt.status()};
}
