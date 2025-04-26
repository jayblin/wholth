#include "wholth/entity/food.hpp"
#include "fmt/color.h"
#include "fmt/core.h"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/transaction.hpp"
#include "sqlw/utils.hpp"
#include "wholth/concepts.hpp"
#include "wholth/entity/locale.hpp"
#include "wholth/entity/nutrient.hpp"
#include "wholth/pager.hpp"
#include "wholth/status.hpp"
#include "wholth/utils.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <exception>
#include <gsl/util>
#include <optional>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>
#include "gsl/assert"

using SC = wholth::status::Code;
using Condition = wholth::status::Condition;
using bindable_t = sqlw::Statement::bindable_t;
template <size_t N>
using bindable_array_t = std::array<bindable_t, N>;

/* static bool check_stmt(sqlw::Statement& stmt) noexcept */
/* { */
/* 	return sqlw::status::Condition::OK != stmt.status(); */
/* } */

constexpr auto count_spaces(const std::string_view& str) -> size_t
{
    size_t count = 0;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == ' ')
        {
            count++;
        }
    }

    return count;
}

static bool check_date(std::string_view value)
{
    constexpr auto s = std::string_view{"YYYY-MM-DDTHH:MM:SS"}.size();
    constexpr std::array<size_t, s - 5> digits = {
        0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18};
    constexpr std::array<std::tuple<size_t, const char>, s - digits.size()>
        non_digits{{
            {4, '-'},
            {7, '-'},
            {10, 'T'},
            {13, ':'},
            {16, ':'},
        }};

    if (value.size() != s)
    {
        return false;
    }

    for (const auto idx : digits)
    {
        if (!std::isdigit(value[idx]))
        {
            return false;
        }
    }

    for (const auto& e : non_digits)
    {
        if (value[std::get<size_t>(e)] != std::get<const char>(e))
        {
            return false;
        }
    }

    return true;
}

std::error_code wholth::insert_food(
    const wholth::entity::editable::Food& food,
    const wholth::entity::locale::id_t locale_id,
    std::string& result_id,
    sqlw::Connection& con) noexcept
{
    if (locale_id.size() == 0 || !sqlw::utils::is_numeric(locale_id))
    {
        return SC::INVALID_LOCALE_ID;
    }

    /* sqlw::Statement stmt {&con}; */

    sqlw::Transaction transaction{&con};

    const std::string now = wholth::utils::current_time_and_date();

    {
        const std::array<bindable_t, 5> params {{
           {now, sqlw::Type::SQL_TEXT},
           {locale_id, sqlw::Type::SQL_INT},
           {food.title, sqlw::Type::SQL_TEXT},
           {food.description, sqlw::Type::SQL_TEXT}
        }};

        const auto ec = transaction(
            "INSERT INTO food (created_at) VALUES (:1) RETURNING id;"
            "INSERT INTO food_localisation (food_id,locale_id,title,description) "
            "VALUES (last_insert_rowid(),:1,trim(:2),:3)",
            [&result_id](auto e) { result_id = e.column_value; },
            params);

        return ec;
    }

    // todo add calories calculation here.

    /* return ec; */
}

/* std::error_code wholth::entity::food::insert( */
/*     const Editable& food, */
/*     const wholth::entity::locale::id_t locale_id, */
/*     id_t& new_food_id, */
/*     sqlw::Connection& con */
/* ) noexcept */
/* { */
/*     if (locale_id.size() == 0 || !sqlw::utils::is_numeric(locale_id)) */
/*     { */
/*         return SC::INVALID_LOCALE_ID; */
/*     } */

/*     /1* sqlw::Statement stmt {&con}; *1/ */

/*     sqlw::Transaction transaction{&con}; */

/*     const std::string now = wholth::utils::current_time_and_date(); */

/*     { */
/*         const std::array<bindable_t, 5> params {{ */
/*            {now, sqlw::Type::SQL_TEXT}, */
/*            {locale_id, sqlw::Type::SQL_INT}, */
/*            {food.title, sqlw::Type::SQL_TEXT}, */
/*            {food.description, sqlw::Type::SQL_TEXT} */
/*         }}; */

/*         const auto ec = transaction( */
/*             "INSERT INTO food (created_at) VALUES (:1) RETURNING id;" */
/*             "INSERT INTO food_localisation (food_id,locale_id,title,description) " */
/*             "VALUES (last_insert_rowid(),:1,trim(:2),:3)", */
/*             [&new_food_id](auto e) { new_food_id = e.column_value; }, */
/*             params); */

/*         return ec; */
/*     } */

/*     // todo add calories calculation here. */

/*     /1* return ec; *1/ */
/* } */

wholth::UpdateFoodStatus wholth::update_food(
    const wholth::entity::editable::Food& food,
    const wholth::entity::locale::id_t locale_id,
    sqlw::Connection& con) noexcept
{
    wholth::UpdateFoodStatus status{};

    if (locale_id.size() == 0 || !sqlw::utils::is_numeric(locale_id))
    {
        status.ec = SC::INVALID_LOCALE_ID;
    }
    else if (food.id.size() == 0 || !sqlw::utils::is_numeric(food.id))
    {
        status.ec = SC::INVALID_FOOD_ID;
    }

    if (!(food.title.size() > 0) ||
        food.title.size() == count_spaces(food.title))
    {
        status.title = SC::EMPTY_FOOD_TITLE;
    }
    else if (food.title == wholth::utils::NIL)
    {
        status.title = SC::UNCHANGED_FOOD_TITLE;
    }

    if (food.description == wholth::utils::NIL)
    {
        status.description = SC::UNCHANGED_FOOD_DESCRIPTION;
    }

    if ((status.title && status.description) || status.ec)
    {
        return status;
    }

    sqlw::Transaction transaction{&con};
    std::vector<bindable_t> params{};

    std::stringstream ss;
    ss << "UPDATE food_localisation SET ";

    if (!status.title)
    {
        params.emplace_back(bindable_t{food.title, sqlw::Type::SQL_TEXT});
        ss << fmt::format("title = trim(:{})", params.size());
    }

    if (!status.description)
    {
        if (params.size() > 0)
        {
            ss << ",";
        }

        params.emplace_back(bindable_t{food.description, sqlw::Type::SQL_TEXT});
        ss << fmt::format("description = :{} ", params.size());
    }

    ss << fmt::format(
        "WHERE food_id = :{} AND locale_id = :{}",
        params.size() + 1,
        params.size() + 2);

    params.emplace_back(bindable_t{food.id, sqlw::Type::SQL_INT});
    params.emplace_back(bindable_t{locale_id, sqlw::Type::SQL_INT});

    status.ec = transaction(ss.str(), params);

    return status;
}

std::error_code wholth::remove_food(
    wholth::entity::food::id_t food_id,
    sqlw::Connection& con) noexcept
{
    if (food_id.size() == 0 || !sqlw::utils::is_numeric(food_id))
    {
        return SC::INVALID_FOOD_ID;
    }

    return sqlw::Transaction{&con}(
        "DELETE FROM food WHERE id = :1",
        std::array<bindable_t, 1>{std::pair{food_id, sqlw::Type::SQL_INT}});
}

std::error_code wholth::entity::food::fill_details(
    food::id_t id,
    wholth::entity::locale::id_t locale_id,
    wholth::BufferView<wholth::utils::LengthContainer>& details,
    sqlw::Connection& con
) noexcept
{
    if (sqlw::status::Condition::OK != con.status()) {
        return con.status();
    }

    if (id.size() == 0 || !sqlw::utils::is_numeric(id)) {
        return SC::INVALID_FOOD_ID;
    }

    if (locale_id.size() == 0 || !sqlw::utils::is_numeric(locale_id))
    {
        return SC::INVALID_LOCALE_ID;
    }

    sqlw::Statement stmt{&con};

    std::stringstream ss{};
    wholth::utils::LengthContainer lc{
        1 // description
    };

    stmt.prepare("SELECT "
                 "COALESCE(fl.description, '[N/A]') AS description "
                 "FROM food f "
                 "LEFT JOIN food_localisation fl "
                 "ON fl.food_id = f.id AND fl.locale_id = :1 "
                 "LEFT JOIN recipe_step rs "
                 "ON rs.recipe_id = f.id "
                 "WHERE f.id = :2")
        .bind(1, locale_id, sqlw::Type::SQL_INT)
        .bind(2, id, sqlw::Type::SQL_INT)
        .exec([&ss, &lc](sqlw::Statement::ExecArgs e) {
            ss << e.column_value;
            lc.add(e.column_value.size());
        });

    if (wholth::status::Condition::OK != stmt.status())
    {
        return stmt.status();
    }

    if (ss.rdbuf()->in_avail() == 0)
    {
        return SC::ENTITY_NOT_FOUND;
    }

    details.buffer = ss.str();
    details.view = std::move(lc);

    return SC::OK;
}


std::error_code wholth::entity::food::fill_nutrients(
    food::id_t food_id,
    /* std::span<Nutrient> nutrients, */
    wholth::entity::locale::id_t locale_id,
    wholth::BufferView<wholth::utils::LengthContainer>& nutrients,
    size_t size,
    sqlw::Connection& con
    ) noexcept
{
    assert(0 <= size);

    if (sqlw::status::Condition::OK != con.status()) {
        return con.status();
    }

    if (food_id.size() == 0 || !sqlw::utils::is_numeric(food_id)) {
        return SC::INVALID_FOOD_ID;
    }

    if (locale_id.size() == 0 || !sqlw::utils::is_numeric(locale_id))
    {
        return SC::INVALID_LOCALE_ID;
    }

    sqlw::Statement stmt{&con};

    std::stringstream ss{};
    wholth::utils::LengthContainer lc{
        5 // id, title, value, unit, user_value
        /* * details.size()}; */
        * size};
    const std::string_view sql = R"sql(
			SELECT
				n.id,
                COALESCE(nl.title, '[N/A]'),
                ROUND(fn.value, 3),
                n.unit,
                0
			FROM food_nutrient fn
            INNER JOIN nutrient n
                ON n.id = fn.nutrient_id
            LEFT JOIN nutrient_localisation nl
                ON nl.nutrient_id = n.id AND nl.locale_id = ?2
            WHERE
                fn.food_id = ?1
                AND fn.value > 0
            ORDER BY n.position ASC, nl.title ASC
            LIMIT ?3
		)sql";

    /* stmt.prepare(sql) */
    /*     .bind(1, food_id, sqlw::Type::SQL_INT) */
    /*     .bind(2, locale_id, sqlw::Type::SQL_INT) */
    /*     // todo check size */
    /*     /1* .bind(3, static_cast<int>(details.size())); *1/ */
    /*     .bind(3, static_cast<int>(size)); */
    const auto ec = stmt(
        sql,
        [&ss, &lc /*, &i */](sqlw::Statement::ExecArgs e) {
            /* if (0 == i % e.column_count) { */
            /* 	fmt::print("\n"); */
            /* } */
            /* i++; */
            /* fmt::print("{}: {}\n", e.column_name, e.column_value); */

            ss << e.column_value;
            lc.add(e.column_value.size());
        },
        std::tuple{
            bindable_t{food_id, sqlw::Type::SQL_INT},
            bindable_t{locale_id, sqlw::Type::SQL_INT},
            static_cast<int>(size)
        }
    );

    /* return SC{}; */
    /* std::cout << */
    /*     "AAAAAAAAAAAAAAAAAAA\n" */
    /*     << food_id << '\n' */
    /*     << locale_id << '\n' */
    /*     << nutrients.size() << '\n' */
    /*     << stmt.status() << '\n'; */

    if (sqlw::status::Condition::OK != ec)
    {
        return ec;
        /* return SC::SQL_STATEMENT_ERROR; */
    }

    if (ss.rdbuf()->in_avail() == 0)
    {
        return SC::ENTITY_NOT_FOUND;
    }

    nutrients.buffer = ss.str();
    nutrients.view = std::move(lc);

    /* for (size_t j = 0; j < details.size(); j++) */
    /* { */
    /*     auto& entry = nutrients[j]; */

    /*     entry.id = itr.next(buffer); */
    /*     entry.title = itr.next(buffer); */
    /*     entry.value = itr.next(buffer); */
    /*     entry.unit = itr.next(buffer); */
    /*     entry.user_value = itr.next(buffer); */
    /* } */

    /* return SC::NO_ERROR; */
    return ec;
}






















/* std::error_code wholth::add_nutrients( */
/*     wholth::entity::food::id_t food_id, */
/*     std::span<const wholth::entity::editable::food::Nutrient> nutrients, */
/*     sqlw::Connection& con) noexcept */
/* { */
/*     if (!(nutrients.size() > 0)) */
/*     { */
/*         return SC::OK; */
/*     } */

/*     wholth::utils::IndexSequence<3, 1> idxs{}; */
/*     std::stringstream ss; */

/*     ss << "INSERT INTO food_nutrient (food_id,nutrient_id,value) " */
/*           "VALUES "; */

/*     for (const auto& nutrient : nutrients) */
/*     { */
/*         if (nutrient.value.length() == 0) */
/*         { */
/*             break; */
/*         } */

/*         ss << fmt::format("(:{},:{},:{}),", idxs[0], idxs[1], idxs[2]); */

/*         idxs.advance(); */
/*     } */

/*     std::string sql = ss.str(); */

/*     sqlw::Transaction transaction{&con}; */

/*     /1* stmt.prepare(sql.substr(0, sql.size() - 1)); *1/ */
/*     std::vector<std::pair<std::string_view, sqlw::Type>> args{}; */
/*     args.reserve(nutrients.size() * 3); */

/*     gsl::index idx = 0; */

/*     for (const auto& nutrient : nutrients) */
/*     { */
/*         if (nutrient.value.length() == 0) */
/*         { */
/*             break; */
/*         } */

/*         /1* stmt.bind(idxs[0], food_id, sqlw::Type::SQL_INT); *1/ */
/*         /1* stmt.bind(idxs[1], nutrient.id, sqlw::Type::SQL_INT); *1/ */
/*         /1* args[0] = {food_id, }; *1/ */
/*         /1* args.push_back({food_id, sqlw::Type::SQL_INT}); *1/ */
/*         /1* args.push_back({nutrient.id, sqlw::Type::SQL_INT}); *1/ */
/*         /1* args.push_back({nutrient.value, sqlw::Type::SQL_DOUBLE}); *1/ */
/*         args[idx++] = {food_id, sqlw::Type::SQL_INT}; */
/*         args[idx++] = {nutrient.id, sqlw::Type::SQL_INT}; */
/*         args[idx++] = {nutrient.value, sqlw::Type::SQL_DOUBLE}; */

/*         try */
/*         { */
/*             /1* stmt.bind(idxs[2], nutrient.value, sqlw::Type::SQL_DOUBLE); *1/ */
/*             /1* args.push_back({nutrient.value, sqlw::Type::SQL_DOUBLE}); *1/ */
/*         } */
/*         catch (const std::invalid_argument& e) */
/*         { */
/*             break; */
/*         }; */

/*         /1* idxs.advance(); *1/ */
/*     } */

/*     /1* stmt.exec(); *1/ */
/*     return transaction( */
/*         sql.substr(0, sql.size() - 1), std::span{args.begin(), args.size()}); */
/* } */

/* wholth::SpanResult wholth::remove_nutrients( */
/*     wholth::entity::food::id_t food_id, */
/*     std::span<const wholth::entity::editable::food::Nutrient> nutrients, */
/*     sqlw::Connection& con) noexcept */
/* { */
/*     const std::string now = wholth::utils::current_time_and_date(); */
/*     sqlw::Statement stmt{&con}; */

/*     if (!(nutrients.size() > 0)) */
/*     { */
/*         return {0, SC::OK}; */
/*     } */

/*     size_t removed_count = 0; */
/*     for (const auto& nutrient : nutrients) */
/*     { */
/*         stmt.prepare("DELETE FROM food_nutrient WHERE food_id = :1 AND " */
/*                      "nutrient_id = :2") */
/*             .bind(1, food_id, sqlw::Type::SQL_INT) */
/*             .bind(2, nutrient.id, sqlw::Type::SQL_INT) */
/*             .exec(); */

/*         if (sqlw::status::Condition::OK != stmt.status()) */
/*         { */
/*             break; */
/*         } */

/*         removed_count++; */
/*     } */

/*     return {removed_count, stmt.status()}; */
/* } */

/* // @todo - подумать как показывать ошибки. */
/* wholth::SpanResult wholth::update_nutrients( */
/*     wholth::entity::food::id_t food_id, */
/*     std::span<const wholth::entity::editable::food::Nutrient> nutrients, */
/*     sqlw::Connection& con) noexcept */
/* { */
/*     if (!(nutrients.size() > 0)) */
/*     { */
/*         return {0, SC::OK}; */
/*     } */

/*     const std::string now = wholth::utils::current_time_and_date(); */
/*     sqlw::Statement stmt{&con}; */

/*     size_t updated_count = 0; */
/*     for (const auto& nutrient : nutrients) */
/*     { */
/*         stmt.prepare("UPDATE food_nutrient SET value = :1 " */
/*                      "WHERE nutrient_id = :2 AND food_id = :3") */
/*             .bind(1, nutrient.value) */
/*             .bind(2, nutrient.id) */
/*             .bind(3, food_id) */
/*             .exec(); */

/*         if (sqlw::status::Condition::OK != stmt.status()) */
/*         { */
/*             break; */
/*         } */

/*         updated_count++; */
/*     } */

/*     return {updated_count, stmt.status()}; */
/* } */

/* std::error_code wholth::add_steps( */
/*     wholth::entity::food::id_t food_id, */
/*     wholth::entity::locale::id_t locale_id, */
/*     std::span<const wholth::entity::editable::food::RecipeStep> steps, */
/*    sqlw::Connection& con ) noexcept */
/* { */
/*     if (!(steps.size() > 0)) */
/*     { */
/*         return SC::OK; */
/*     } */

/*     sqlw::Statement stmt{&con}; */


/*     std::string recipe_step_id; */

/*     auto ec = sqlw::Transaction{&con}( */
/*         "INSERT INTO recipe_step (recipe_id,seconds) VALUES (:1,:2) RETURNING recipe_id", */
/*         [&recipe_step_id] (auto e) { recipe_step_id = e.column_value; }, */
/*         std::array<bindable_t, 2>{{ */
/*             {food_id, sqlw::Type::SQL_TEXT}, */
/*             {steps[0].seconds, sqlw::Type::SQL_INT}, */
/*             }} */
/*     ); */

/*     if (Condition::OK != ec) { */
/*         return ec; */
/*     } */

/*     ec = sqlw::Transaction{&con}( */
/*         "INSERT INTO recipe_step_localisation " */
/*          "(recipe_step_id,locale_id,description) " */
/*          "VALUES (:1,:2,:3) ", */
/*         [&recipe_step_id] (auto e) { recipe_step_id = e.column_value; }, */
/*         std::array<bindable_t, 3>{{ */
/*             {recipe_step_id, sqlw::Type::SQL_TEXT}, */
/*             {locale_id, sqlw::Type::SQL_INT}, */
/*             {steps[0].description, sqlw::Type::SQL_TEXT}, */
/*             }} */
/*     ); */

/*     return ec; */
/* } */

/* void wholth::remove_steps( */
/*     std::span<const wholth::entity::editable::food::RecipeStep> steps, */
/*     sqlw::Connection& con) noexcept */
/* { */
/*     if (!(steps.size() > 0)) */
/*     { */
/*         return; */
/*     } */

/*     sqlw::Statement stmt{&con}; */

/*     // @todo: maybe redo? */
/*     for (const auto& step : steps) */
/*     { */
/*         stmt.prepare("DELETE FROM recipe_step WHERE id = :1") */
/*             .bind(1, step.id, sqlw::Type::SQL_INT) */
/*             .exec(); */

/*         if (sqlw::status::Condition::OK != stmt.status()) */
/*         { */
/*             stmt("ROLLBACK TO remove_steps_pnt"); */
/*             return; */
/*         } */

/*         stmt("RELEASE remove_steps_pnt"); */
/*     } */
/* } */

/* void update_steps( */
/*     std::span<const wholth::entity::editable::food::RecipeStep> steps, */
/*     sqlw::Connection& con) */
/* { */
/*     using wholth::utils::NIL; */

/*     if (!(steps.size() > 0)) */
/*     { */
/*         return; */
/*     } */

/*     sqlw::Statement stmt{&con}; */
/*     std::stringstream ss; */

/*     for (const auto& step : steps) */
/*     { */
/*         ss.clear(); */

/*         stmt("SAVEPOINT update_steps_pnt"); */
/*         bool _description = step.description != NIL; */
/*         bool _time = step.seconds != NIL; */

/*         stmt.prepare(fmt::format( */
/*             "UPDATE recipe_step_localisation SET description = {}, seconds = " */
/*             "{}", */
/*             _description ? ":1" : "description", */
/*             _time ? ":2" : "seconds")); */

/*         if (_description) */
/*         { */
/*             stmt.bind(1, step.description, sqlw::Type::SQL_TEXT); */
/*         } */
/*         if (_time) */
/*         { */
/*             stmt.bind(2, step.seconds, sqlw::Type::SQL_INT); */
/*         } */

/*         stmt.exec(); */

/*         if (sqlw::status::Condition::OK != stmt.status()) */
/*         { */
/*             stmt("ROLLBACK TO update_steps_pnt"); */
/*             continue; */
/*         } */

/*         stmt("RELEASE update_steps_pnt"); */
/*     } */
/* } */

/* void wholth::add_ingredients( */
/*     wholth::entity::recipe_step::id_t recipe_step_id, */
/*     std::span<const wholth::entity::editable::food::Ingredient> foods, */
/*     sqlw::Connection& con) noexcept */
/* { */
/*     sqlw::Statement stmt{&con}; */
/*     stmt("SAVEPOINT add_foods_to_step_pnt"); */

/*     if (!(foods.size() > 0)) */
/*     { */
/*         return; */
/*     } */

/*     std::stringstream ss; */
/*     wholth::utils::IndexSequence<3, 1> idxs; */

/*     ss << "INSERT INTO recipe_step_food " */
/*           "(recipe_step_id,food_id,canonical_mass) " */
/*           "VALUES "; */

/*     for (const auto& step_food : foods) */
/*     { */
/*         if (step_food.food_id.length() == 0) */
/*         { */
/*             break; */
/*         } */

/*         ss << fmt::format("(:{},:{},:{}),", idxs[0], idxs[1], idxs[2]); */

/*         idxs.advance(); */
/*     } */

/*     std::string sql = ss.str(); */

/*     stmt.prepare(sql.substr(0, sql.size() - 1)); */

/*     idxs.reset(); */

/*     for (const auto& step_food : foods) */
/*     { */
/*         if (step_food.food_id.length() == 0) */
/*         { */
/*             break; */
/*         } */

/*         stmt.bind(idxs[0], recipe_step_id, sqlw::Type::SQL_INT); */
/*         stmt.bind(idxs[1], step_food.food_id, sqlw::Type::SQL_INT); */
/*         stmt.bind(idxs[2], step_food.canonical_mass, sqlw::Type::SQL_DOUBLE); */

/*         idxs.advance(); */
/*     } */

/*     // todo add calorie recalc here */

/*     stmt.exec(); */
/* } */

/* void wholth::update_ingredients( */
/*     wholth::entity::recipe_step::id_t recipe_step_id, */
/*     std::span<const wholth::entity::editable::food::Ingredient> foods, */
/*     sqlw::Connection& con) noexcept */
/* { */
/*     sqlw::Statement stmt{&con}; */

/*     if (!(foods.size() > 0)) */
/*     { */
/*         return; */
/*     } */

/*     for (const auto& ing : foods) */
/*     { */
/*         stmt("SAVEPOINT update_ingreddients_pnt"); */
/*         stmt.prepare("UPDATE recipe_step_food SET canonical_mass = ?1 " */
/*                      "WHERE recipe_step_id = ?2 AND food_id = ?3") */
/*             .bind(1, ing.canonical_mass, sqlw::Type::SQL_DOUBLE) */
/*             .bind(2, recipe_step_id, sqlw::Type::SQL_INT) */
/*             .bind(3, ing.food_id, sqlw::Type::SQL_INT) */
/*             .exec(); */

/*         if (sqlw::status::Condition::OK != stmt.status()) */
/*         { */
/*             stmt("ROLLBACK TO update_ingreddients_pnt"); */
/*             continue; */
/*         } */

/*         stmt("RELEASE update_ingreddients_pnt"); */
/*     } */

/*     // todo add calorie recalc here */
/* } */

/* SC wholth::recalc_nutrients( */
/*     wholth::entity::food::id_t food_id, */
/*     sqlw::Connection& con) noexcept */
/* { */
/*     sqlw::Statement stmt{&con}; */

/*     stmt("SAVEPOINT food_recalc_nutrients_pnt"); */

/*     /1* stmt.prepare(R"sql( *1/ */
/*     /1* 	WITH cte( *1/ */
/*     /1* 		nutrient_id, *1/ */
/*     /1* 		summed_values *1/ */
/*     /1* 	) AS ( *1/ */
/*     /1* 		SELECT *1/ */
/*     /1* 			n.id, *1/ */
/*     /1* 			SUM(fn.value) *1/ */
/*     /1* 		FROM recipe_step rs *1/ */
/*     /1* 		LEFT JOIN recipe_step_food rsf *1/ */
/*     /1* 			ON rsf.recipe_step_id = rs.id *1/ */
/*     /1* 		INNER JOIN food_nutrient fn *1/ */
/*     /1* 			ON fn.food_id = rsf.food_id *1/ */
/*     /1* 		LEFT JOIN nutrient n *1/ */
/*     /1* 			ON n.id = fn.nutrient_id *1/ */
/*     /1* 		WHERE rs.recipe_id = ?1 *1/ */
/*     /1* 		GROUP BY n.id *1/ */
/*     /1* 		ORDER BY n.position ASC *1/ */
/*     /1* 		LIMIT 4 *1/ */
/*     /1* 	) *1/ */
/*     /1* 	INSERT OR REPLACE INTO food_nutrient *1/ */
/*     /1* 		(nutrient_id, food_id, value) *1/ */
/*     /1* 	SELECT *1/ */
/*     /1* 		cte.nutrient_id, *1/ */
/*     /1* 		?1, *1/ */
/*     /1* 		cte.summed_values *1/ */
/*     /1* 	FROM cte *1/ */
/*     /1* )sql") *1/ */
/*     stmt.prepare(R"sql( */
/* 			WITH RECURSIVE */
/* 			recipe_tree( */
/* 				lvl, */
/* 				recipe_id, */
/* 				recipe_mass, */
/* 				recipe_ingredient_count, */
/* 				ingredient_id, */
/* 				ingredient_mass, */
/* 				ingredient_weight */
/* 			) AS ( */
/* 				SELECT */
/* 					root.lvl, */
/* 					root.recipe_id, */
/* 					root.recipe_mass, */
/* 					root.recipe_ingredient_count, */
/* 					root.ingredient_id, */
/* 					root.ingredient_mass, */
/* 					root.ingredient_weight */
/* 				FROM recipe_tree_node root */
/* 				WHERE root.recipe_id = ?1 */
/* 				UNION */
/* 				SELECT */
/* 					rt.lvl + 1, */
/* 					node.recipe_id, */
/* 					node.recipe_mass, */
/* 					node.recipe_ingredient_count, */
/* 					node.ingredient_id, */
/* 					node.ingredient_mass, */
/* 					node.ingredient_mass / node.recipe_mass * rt.ingredient_weight */
/* 				FROM recipe_tree rt */
/* 				INNER JOIN recipe_tree_node node */
/* 					ON node.recipe_id = rt.ingredient_id */
/* 				ORDER BY 1 DESC */
/* 			), */
/* 			calced_nutrients( */
/* 				nutrient_id, */
/* 				nutrient_position, */
/* 				sum_weight, */
/* 				sum_values */
/* 			) AS ( */
/* 				SELECT */
/* 					n.id, */
/* 					n.position, */
/* 					SUM(rt.ingredient_weight) sum_weight, */
/* 					SUM(fn.value * rt.ingredient_weight) * 100 / root_recipe_info.recipe_mass */
/* 				FROM recipe_tree rt */
/* 				LEFT JOIN recipe_info ingredient_info */
/* 					ON ingredient_info.recipe_id = rt.ingredient_id */
/* 				INNER JOIN food_nutrient fn */
/* 					ON fn.food_id = rt.ingredient_id */
/* 				LEFT JOIN nutrient n */
/* 					ON n.id = fn.nutrient_id */
/* 				LEFT JOIN recipe_info root_recipe_info */
/* 					ON root_recipe_info.recipe_id = ?1 */
/* 				LEFT JOIN food_nutrient root_food_nutrient */
/* 					ON root_food_nutrient.food_id = ?1 AND root_food_nutrient.nutrient_id = n.id */
/* 				-- So that ingredients that are also recipes will not have their */
/* 				-- user specified nutrient values summated. */
/* 				WHERE ingredient_info.recipe_id IS NULL */
/* 				GROUP BY n.id */
/* 				HAVING sum_weight = 1 */
/* 				ORDER BY n.position ASC */
/* 			) */
/* 		INSERT OR REPLACE INTO food_nutrient */
/* 			(nutrient_id, food_id, value) */
/* 		SELECT */
/* 			cn.nutrient_id, */
/* 			?1, */
/* 			cn.sum_values */
/* 		FROM calced_nutrients cn */
/* 		WHERE cn.nutrient_position < 4 */
/* 		)sql") */
/*         .bind(1, food_id, sqlw::Type::SQL_INT) */
/*         .exec(); */

/*     if (sqlw::status::Condition::OK != stmt.status()) */
/*     { */
/*         stmt("ROLLBACK TO food_recalc_nutrients_pnt"); */
/*         return SC::SQL_STATEMENT_ERROR; */
/*     } */

/*     stmt("RELEASE food_recalc_nutrients_pnt"); */

/*     return stmt.status(); */
/* } */

/* void wholth::remove_ingredients( */
/*     wholth::entity::recipe_step::id_t recipe_step_id, */
/*     std::span<const wholth::entity::editable::food::Ingredient> foods, */
/*     sqlw::Connection& con) noexcept */
/* { */
/*     if (!(foods.size() > 0)) */
/*     { */
/*         return; */
/*     } */

/*     sqlw::Statement stmt{&con}; */

/*     // todo: maybe redo? */
/*     for (size_t i = 0; i < foods.size(); i++) */
/*     { */
/*         stmt("SAVEPOINT remove_ingreddients_pnt"); */

/*         stmt.prepare("DELETE FROM recipe_step_food " */
/*                      "WHERE recipe_step_id = :1 AND food_id = :2") */
/*             .bind(1, recipe_step_id, sqlw::Type::SQL_INT) */
/*             .bind(2, foods[i].food_id, sqlw::Type::SQL_INT) */
/*             .exec(); */

/*         if (sqlw::status::Condition::OK != stmt.status()) */
/*         { */
/*             stmt("ROLLBACK TO remove_ingreddients_pnt"); */
/*         } */
/*         else */
/*         { */
/*             stmt("RELEASE remove_ingreddients_pnt"); */
/*         } */
/*     } */

/*     // todo add calorie recalc here */
/* } */

/* SC wholth::expand_food( */
/*     wholth::entity::expanded::Food& food, */
/*     std::string& buffer, */
/*     wholth::entity::food::id_t id, */
/*     wholth::entity::locale::id_t locale_id, */
/*     sqlw::Connection* con) noexcept */
/* { */
/*     if (locale_id.size() == 0 || !sqlw::utils::is_numeric(locale_id)) */
/*     { */
/*         return SC::INVALID_LOCALE_ID; */
/*     } */

/*     sqlw::Statement stmt{con}; */

/*     std::stringstream ss{}; */
/*     wholth::utils::LengthContainer itr{ */
/*         5 // id, title, description, calories, prep_time */
/*     }; */

/*     stmt.prepare("SELECT " */
/*                  "f.id AS id, " */
/*                  "COALESCE(fl.title, '[N/A]') AS title, " */
/*                  "COALESCE(fl.description, '[N/A]') AS description, " */
/*                  "COALESCE(" */
/*                  "seconds_to_readable_time(rs.seconds)," */
/*                  "'[N/A]'" */
/*                  ") AS preparation_time " */
/*                  "FROM food f " */
/*                  "LEFT JOIN food_localisation fl " */
/*                  "ON fl.food_id = f.id AND fl.locale_id = :1 " */
/*                  "LEFT JOIN recipe_step rs " */
/*                  "ON rs.recipe_id = f.id " */
/*                  "WHERE f.id = :2") */
/*         .bind(1, locale_id, sqlw::Type::SQL_INT) */
/*         .bind(2, id, sqlw::Type::SQL_INT) */
/*         .exec([&ss, &itr](sqlw::Statement::ExecArgs e) { */
/*             ss << e.column_value; */
/*             itr.add(e.column_value.size()); */
/*         }); */

/*     SC rc = check_stmt(stmt); */

/*     if (!rc) */
/*     { */
/*         return rc; */
/*     } */

/*     if (ss.rdbuf()->in_avail() == 0) */
/*     { */
/*         return SC::ENTITY_NOT_FOUND; */
/*     } */

/*     buffer = ss.str(); */

/*     food.id = itr.next(buffer); */
/*     food.title = itr.next(buffer); */
/*     food.description = itr.next(buffer); */
/*     food.preparation_time = itr.next(buffer); */

/*     return rc; */
/* } */

/* auto wholth::list_steps( */
/*     std::span<wholth::entity::expanded::food::RecipeStep> steps, */
/*     std::string& buffer, */
/*     wholth::entity::food::id_t food_id, */
/*     wholth::entity::locale::id_t locale_id, */
/*     sqlw::Connection* con) noexcept -> StatusCode */
/* { */
/*     if (!(locale_id.size() >= 1) || !sqlw::utils::is_numeric(locale_id)) */
/*     { */
/*         return SC::INVALID_LOCALE_ID; */
/*     } */

/*     sqlw::Statement stmt{con}; */

/*     std::stringstream ss{}; */
/*     wholth::utils::LengthContainer itr{ */
/*         3 // id, time, description */
/*         * steps.size()}; */
/*     stmt.prepare("SELECT " */
/*                  "rs.id AS id," */
/*                  "COALESCE(" */
/*                  "seconds_to_readable_time(rs.seconds)," */
/*                  "'[N/A]'" */
/*                  ") AS time, " */
/*                  "COALESCE(rsl.description, '[N/A]') AS description " */
/*                  "FROM recipe_step rs " */
/*                  "LEFT JOIN recipe_step_localisation rsl " */
/*                  "ON rsl.recipe_step_id = rs.id AND locale_id = ?1 " */
/*                  "WHERE rs.recipe_id = ?2 " */
/*                  "ORDER BY rs.id ASC " */
/*                  "LIMIT ?3 ") */
/*         .bind(1, locale_id, sqlw::Type::SQL_INT) */
/*         .bind(2, food_id, sqlw::Type::SQL_INT) */
/*         // todo: check that size is not bigger than int. */
/*         .bind(3, static_cast<int>(steps.size())); */

/*     stmt([&ss, &itr](sqlw::Statement::ExecArgs e) { */
/*         ss << e.column_value; */
/*         itr.add(e.column_value.size()); */
/*     }); */

/*     if (sqlw::status::Condition::OK != stmt.status()) */
/*     { */
/*         return SC::SQL_STATEMENT_ERROR; */
/*     } */

/*     if (ss.rdbuf()->in_avail() == 0) */
/*     { */
/*         return SC::ENTITY_NOT_FOUND; */
/*     } */

/*     buffer = ss.str(); */

/*     for (size_t j = 0; j < steps.size(); j++) */
/*     { */
/*         auto& entry = steps[j]; */

/*         entry.id = itr.next(buffer); */
/*         entry.time = itr.next(buffer); */
/*         entry.description = itr.next(buffer); */
/*     } */

/*     return SC::NO_ERROR; */
/* } */

/* auto wholth::list_ingredients( */
/*     std::span<wholth::entity::expanded::food::Ingredient> ingredients, */
/*     std::string& buffer, */
/*     wholth::entity::food::id_t recipe_id, */
/*     wholth::entity::locale::id_t locale_id, */
/*     sqlw::Connection* con) noexcept -> SC */
/* { */
/*     if (!(locale_id.size() >= 1) || !sqlw::utils::is_numeric(locale_id)) */
/*     { */
/*         return SC::INVALID_LOCALE_ID; */
/*     } */

/*     sqlw::Statement stmt{con}; */

/*     std::stringstream ss{}; */
/*     wholth::utils::LengthContainer itr{ */
/*         4 // food_id, title, canonica_mass, ingredient_count */
/*         * ingredients.size()}; */
/*     stmt.prepare("SELECT " */
/*                  "f.id AS food_id, " */
/*                  "COALESCE(fl.title, '[N/A]') AS title, " */
/*                  "rsf.canonical_mass AS canonical_mass, " */
/*                  "( " */
/*                  "SELECT COUNT(rs1.id) " */
/*                  "FROM recipe_step rs1 " */
/*                  "WHERE rs1.recipe_id = f.id " */
/*                  ") AS ingredient_count " */
/*                  "FROM recipe_step rs " */
/*                  "LEFT JOIN recipe_step_food rsf " */
/*                  "ON rsf.recipe_step_id = rs.id " */
/*                  "LEFT JOIN food f " */
/*                  "ON f.id = rsf.food_id " */
/*                  "LEFT JOIN food_localisation fl " */
/*                  "ON fl.food_id = f.id AND fl.locale_id = :1 " */
/*                  "WHERE rs.recipe_id = :2 " */
/*                  "LIMIT :3 ") */
/*         .bind(1, locale_id, sqlw::Type::SQL_INT) */
/*         .bind(2, recipe_id, sqlw::Type::SQL_INT) */
/*         // todo check size */
/*         .bind(3, static_cast<int>(ingredients.size())); */

/*     stmt([&ss, &itr](sqlw::Statement::ExecArgs e) { */
/*         ss << e.column_value; */
/*         itr.add(e.column_value.size()); */
/*     }); */

/*     if (sqlw::status::Condition::OK != stmt.status()) */
/*     { */
/*         return SC::SQL_STATEMENT_ERROR; */
/*     } */

/*     if (ss.rdbuf()->in_avail() == 0) */
/*     { */
/*         return SC::ENTITY_NOT_FOUND; */
/*     } */

/*     buffer = ss.str(); */

/*     for (size_t j = 0; j < ingredients.size(); j++) */
/*     { */
/*         auto& entry = ingredients[j]; */

/*         entry.food_id = itr.next(buffer); */
/*         entry.title = itr.next(buffer); */
/*         entry.canonical_mass = itr.next(buffer); */
/*         entry.ingredient_count = itr.next(buffer); */
/*     } */

/*     return SC::NO_ERROR; */
/* } */

/* auto wholth::list_nutrients( */
/*     std::span<wholth::entity::expanded::food::Nutrient> nutrients, */
/*     std::string& buffer, */
/*     wholth::entity::food::id_t food_id, */
/*     wholth::entity::locale::id_t locale_id, */
/*     sqlw::Connection* con) noexcept -> SC */
/* { */
/*     if (!(locale_id.size() >= 1) || !sqlw::utils::is_numeric(locale_id)) */
/*     { */
/*         return SC::INVALID_LOCALE_ID; */
/*     } */

/*     sqlw::Statement stmt{con}; */

/*     std::stringstream ss{}; */
/*     wholth::utils::LengthContainer itr{ */
/*         5 // id, title, value, unit, user_value */
/*         * nutrients.size()}; */
/*     /1* const std::string_view sql = R"sql( *1/ */
/*     /1* WITH RECURSIVE *1/ */
/*     /1* recipe_tree AS ( *1/ */
/*     /1* SELECT *1/ */
/*     /1* root.lvl, *1/ */
/*     /1* root.recipe_id, *1/ */
/*     /1* root.recipe_mass, *1/ */
/*     /1* root.recipe_ingredient_count, *1/ */
/*     /1* root.ingredient_id, *1/ */
/*     /1* root.ingredient_mass, *1/ */
/*     /1* root.ingredient_weight *1/ */
/*     /1* FROM recipe_tree_node root *1/ */
/*     /1* WHERE root.recipe_id = ?1 *1/ */
/*     /1* UNION *1/ */
/*     /1* SELECT *1/ */
/*     /1* rt.lvl + 1, *1/ */
/*     /1* node.recipe_id, *1/ */
/*     /1* node.recipe_mass, *1/ */
/*     /1* node.recipe_ingredient_count, *1/ */
/*     /1* node.ingredient_id, *1/ */
/*     /1* node.ingredient_mass, *1/ */
/*     /1* node.ingredient_mass / node.recipe_mass * rt.ingredient_weight *1/ */
/*     /1* FROM recipe_tree rt *1/ */
/*     /1* INNER JOIN recipe_tree_node node *1/ */
/*     /1* ON node.recipe_id = rt.ingredient_id *1/ */
/*     /1* ORDER BY 1 DESC *1/ */
/*     /1* ) *1/ */
/*     /1* SELECT *1/ */
/*     /1* id, title, value, unit, user_value *1/ */
/*     /1* FROM ( *1/ */
/*     /1* SELECT *1/ */
/*     /1* fn.nutrient_id AS id, *1/ */
/*     /1* SUM(rt.ingredient_weight) AS sum_weight, *1/ */
/*     /1* COALESCE(nl.title, '[N/A]') AS title, *1/ */
/*     /1* n.unit AS unit, *1/ */
/*     /1* SUM(fn.value * rt.ingredient_weight) * 100 / root_recipe_info.recipe_mass */
/*      * AS value, *1/ */
/*     /1* root_food_nutrient.value AS user_value *1/ */
/*     /1* FROM recipe_tree rt *1/ */
/*     /1* LEFT JOIN recipe_info ingredient_info *1/ */
/*     /1* ON ingredient_info.recipe_id = rt.ingredient_id *1/ */
/*     /1* INNER JOIN food_nutrient fn *1/ */
/*     /1* ON fn.food_id = rt.ingredient_id *1/ */
/*     /1* LEFT JOIN nutrient n *1/ */
/*     /1* ON n.id = fn.nutrient_id *1/ */
/*     /1* LEFT JOIN nutrient_localisation nl *1/ */
/*     /1* ON nl.nutrient_id = n.id AND nl.locale_id = ?2 *1/ */
/*     /1* LEFT JOIN recipe_info root_recipe_info *1/ */
/*     /1* ON root_recipe_info.recipe_id = ?1 *1/ */
/*     /1* LEFT JOIN food_nutrient root_food_nutrient *1/ */
/*     /1* ON root_food_nutrient.food_id = ?1 AND root_food_nutrient.nutrient_id = */
/*      * n.id *1/ */
/*     /1* -- So that ingredients that are also recipes will not have their *1/ */
/*     /1* -- user specified nutrient values summated. *1/ */
/*     /1* WHERE ingredient_info.recipe_id IS NULL *1/ */
/*     /1* GROUP BY fn.nutrient_id *1/ */
/*     /1* HAVING sum_weight = 1 *1/ */
/*     /1* ORDER BY n.position ASC *1/ */
/*     /1* LIMIT ?3 *1/ */
/*     /1* ) *1/ */
/*     /1* )sql"; *1/ */
/*     const std::string_view sql = R"sql( */
/* 			SELECT */
/* 				n.id, */
/*                 COALESCE(nl.title, '[N/A]'), */
/*                 ROUND(fn.value, 3), */
/*                 n.unit, */
/*                 0 */
/* 			FROM food_nutrient fn */
/*             INNER JOIN nutrient n */
/*                 ON n.id = fn.nutrient_id */
/*             LEFT JOIN nutrient_localisation nl */
/*                 ON nl.nutrient_id = n.id AND nl.locale_id = ?2 */
/*             WHERE */
/*                 fn.food_id = ?1 */
/*                 AND fn.value > 0 */
/*             ORDER BY n.position ASC, nl.title ASC */
/*             LIMIT ?3 */
/* 		)sql"; */

/*     stmt.prepare(sql) */
/*         .bind(1, food_id, sqlw::Type::SQL_INT) */
/*         .bind(2, locale_id, sqlw::Type::SQL_INT) */
/*         // todo check size */
/*         .bind(3, static_cast<int>(nutrients.size())); */

/*     /1* size_t i = 0; *1/ */
/*     stmt([&ss, &itr /*, &i *1/](sqlw::Statement::ExecArgs e) { */
/*         /1* if (0 == i % e.column_count) { *1/ */
/*         /1* 	fmt::print("\n"); *1/ */
/*         /1* } *1/ */
/*         /1* i++; *1/ */
/*         /1* fmt::print("{}: {}\n", e.column_name, e.column_value); *1/ */

/*         ss << e.column_value; */
/*         itr.add(e.column_value.size()); */
/*     }); */

/*     /1* return SC{}; *1/ */
/*     /1* std::cout << *1/ */
/*     /1*     "AAAAAAAAAAAAAAAAAAA\n" *1/ */
/*     /1*     << food_id << '\n' *1/ */
/*     /1*     << locale_id << '\n' *1/ */
/*     /1*     << nutrients.size() << '\n' *1/ */
/*     /1*     << stmt.status() << '\n'; *1/ */

/*     if (sqlw::status::Condition::OK != stmt.status()) */
/*     { */
/*         return SC::SQL_STATEMENT_ERROR; */
/*     } */

/*     if (ss.rdbuf()->in_avail() == 0) */
/*     { */
/*         return SC::ENTITY_NOT_FOUND; */
/*     } */

/*     buffer = ss.str(); */

/*     for (size_t j = 0; j < nutrients.size(); j++) */
/*     { */
/*         auto& entry = nutrients[j]; */

/*         entry.id = itr.next(buffer); */
/*         entry.title = itr.next(buffer); */
/*         entry.value = itr.next(buffer); */
/*         entry.unit = itr.next(buffer); */
/*         entry.user_value = itr.next(buffer); */
/*     } */

/*     return SC::NO_ERROR; */
/* } */

/* SC wholth::describe_error( */
/*     SC ec, */
/*     std::string& buffer, */
/*     wholth::entity::locale::id_t locale_id, */
/*     sqlw::Connection* con) noexcept */
/* { */
/*     sqlw::Statement stmt{con}; */
/*     // @todo: think about caching. */
/*     stmt.prepare(fmt::format( */
/*                      "SELECT el.description " */
/*                      "FROM error_localisation el " */
/*                      "WHERE el.error_id = {} AND locale_id = :1", */
/*                      static_cast<int>(ec))) */
/*         .bind(1, locale_id, sqlw::Type::SQL_INT) */
/*         .exec([&buffer](sqlw::Statement::ExecArgs e) { */
/*             buffer = e.column_value; */
/*         }); */

/*     return check_stmt(stmt); */
/* } */

/* std::ostream& operator<<( */
/*     std::ostream& out, */
/*     const wholth::entity::expanded::Food& f) */
/* { */
/*     out << "{\n id: " << f.id << "\n title: " << f.title */
/*         << "\n descriotion: " << f.description */
/*         << "\n preparation_time: " << f.preparation_time << "\n}"; */
/*     return out; */
/* } */

/* std::string_view wholth::view(wholth::StatusCode rc) */
/* { */
/*     switch (rc) */
/*     { */
/*     case wholth::StatusCode::SQL_STATEMENT_ERROR: */
/*         return "SQL_STATEMENT_ERROR"; */
/*     case wholth::StatusCode::NO_ERROR: */
/*         return "NO_ERROR"; */
/*     case wholth::StatusCode::ENTITY_NOT_FOUND: */
/*         return "ENTITY_NOT_FOUND"; */
/*     case wholth::StatusCode::INVALID_LOCALE_ID: */
/*         return "INVALID_LOCALE_ID"; */
/*     case wholth::StatusCode::INVALID_FOOD_ID: */
/*         return "INVALID_FOOD_ID"; */
/*     case wholth::StatusCode::EMPTY_FOOD_TITLE: */
/*         return "EMPTY_FOOD_TITLE"; */
/*     case wholth::StatusCode::UNCHANGED_FOOD_TITLE: */
/*         return "UNCHANGED_FOOD_TITLE"; */
/*     case wholth::StatusCode::UNCHANGED_FOOD_DESCRIPTION: */
/*         return "UNCHANGED_FOOD_DESCRIPTION"; */
/*     case wholth::StatusCode::INVALID_DATE: */
/*         return "INVALID_DATE"; */
/*     case wholth::StatusCode::INVALID_MASS: */
/*         return "INVALID_MASS"; */
/*         /1* default: return "[VIEW_NOT_IMPLEMENTED]"; *1/ */
/*     } */
/* } */

/* auto wholth::log_consumption( */
/*     wholth::entity::food::id_t food_id, */
/*     wholth::entity::consumption_log::mass_t mass, */
/*     wholth::entity::consumption_log::consumed_at_t consumed_at, */
/*     sqlw::Connection& con) noexcept -> SC */
/* { */
/*     sqlw::Statement stmt{&con}; */

/*     if (!(food_id.size() >= 1) || !sqlw::utils::is_numeric(food_id)) */
/*     { */
/*         return SC::INVALID_FOOD_ID; */
/*     } */

/*     if (!(mass.size() >= 1) || !sqlw::utils::is_numeric(mass)) */
/*     { */
/*         return SC::INVALID_MASS; */
/*     } */

/*     if (consumed_at.size() == 0 || */
/*         consumed_at.size() == count_spaces(consumed_at)) */
/*     { */
/*         consumed_at = wholth::utils::current_time_and_date(); */
/*     } */
/*     else if (!check_date(consumed_at)) */
/*     { */
/*         return SC::INVALID_DATE; */
/*     } */

/*     // todo check that is actualy time in needed format */
/*     stmt.prepare(R"sql( */
/* 			INSERT INTO consumption_log (food_id, mass, consumed_at) */
/* 			VALUES */
/* 			(?1, ?2 , ?3) */
/* 		)sql") */
/*         .bind(1, food_id, sqlw::Type::SQL_TEXT) */
/*         .bind(2, mass, sqlw::Type::SQL_DOUBLE) */
/*         .bind(3, consumed_at, sqlw::Type::SQL_TEXT) */
/*         .exec(); */

/*     return check_stmt(stmt); */
/* } */

/* auto wholth::log_consumption( */
/*     wholth::entity::food::id_t food_id, */
/*     wholth::entity::consumption_log::mass_numeric_t mass, */
/*     wholth::entity::consumption_log::consumed_at_t consumed_at, */
/*     sqlw::Connection& con) noexcept -> SC */
/* { */
/*     sqlw::Statement stmt{&con}; */

/*     if (!(food_id.size() >= 1) || !sqlw::utils::is_numeric(food_id)) */
/*     { */
/*         return SC::INVALID_FOOD_ID; */
/*     } */

/*     if (mass < 0) */
/*     { */
/*         return SC::INVALID_MASS; */
/*     } */

/*     if (consumed_at.size() == 0 || */
/*         consumed_at.size() == count_spaces(consumed_at)) */
/*     { */
/*         consumed_at = wholth::utils::current_time_and_date(); */
/*     } */
/*     else if (!check_date(consumed_at)) */
/*     { */
/*         return SC::INVALID_DATE; */
/*     } */

/*     // todo check that is actualy time in needed format */
/*     stmt.prepare(R"sql( */
/* 			INSERT INTO consumption_log (food_id, mass, consumed_at) */
/* 			VALUES */
/* 			(?1, ?2 , ?3) */
/* 		)sql") */
/*         .bind(1, food_id, sqlw::Type::SQL_TEXT) */
/*         .bind(2, mass) */
/*         .bind(3, consumed_at, sqlw::Type::SQL_TEXT) */
/*         .exec(); */

/*     return check_stmt(stmt); */
/* } */
