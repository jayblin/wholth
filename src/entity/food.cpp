#include "wholth/entity/food.hpp"
#include "fmt/color.h"
#include "fmt/core.h"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "sqlw/utils.hpp"
#include "wholth/concepts.hpp"
#include "wholth/entity/locale.hpp"
#include "wholth/entity/nutrient.hpp"
#include "wholth/pager.hpp"
#include "wholth/utils.hpp"
#include <array>
#include <charconv>
#include <exception>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

using SC = wholth::StatusCode;

static SC check_stmt(sqlw::Statement& stmt) noexcept
{
	if (!sqlw::status::is_ok(stmt.status())) {
		return SC::SQL_STATEMENT_ERROR;
	}

	return SC::NO_ERROR;
}

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

static std::string_view create_joins(wholth::entity::locale::id_t locale_id)
{
	/* if (q.locale_id.npos == q.locale_id.find(",")) */
	/* { */
	/* 	return "LEFT JOIN food_localisation fl " */
	/* 	   "ON fl.food_id = f.id AND fl.locale_id IN (?1) "; */
	/* } */

	if (locale_id.size() == 0)
	{
		return "LEFT JOIN food_localisation AS fl "
		   "ON fl.food_id = f.id "
		"LEFT JOIN food_nutrient AS fn "
		"ON fn.food_id = f.id "
		"LEFT JOIN recipe_step rs "
		"ON rs.recipe_id = f.id ";
	}

	return "LEFT JOIN food_localisation AS fl "
	   "ON fl.food_id = f.id AND fl.locale_id = :1 "
		"LEFT JOIN food_nutrient AS fn "
		"ON fn.food_id = f.id "
		"LEFT JOIN recipe_step rs "
		"ON rs.recipe_id = f.id ";
}

static bool is_empty(const wholth::nutrient_filter::Entry& entry)
{
	return !(std::get<wholth::nutrient_filter::Value::index>(entry).length() > 0);
}

static std::string create_where(const wholth::FoodsQuery& q)
{
	using OP = wholth::nutrient_filter::Operation;

	std::stringstream tpl_stream;
	size_t sql_param_idx = q.locale_id.size() == 0 ? 1 : 2;

	for (size_t i = 0; i < q.nutrient_filters.size(); i++)
	{
		const auto& entry = q.nutrient_filters[i];

		if (is_empty(entry))
		{
			break;
		}

		switch (std::get<OP>(entry))
		{
			case OP::EQ: {
				auto idx1 = sql_param_idx;
				sql_param_idx++;
				auto idx2 = sql_param_idx;
				sql_param_idx++;
				tpl_stream << fmt::format(
					"INNER JOIN food_nutrient AS fn{} "
					" ON fn{}.food_id = f.id "
					" AND fn{}.nutrient_id = :{} "
					" AND (fn{}.value <= (:{} + 0.001) AND fn{}.value >= (:{} - 0.001)) ",
					i,
					i,
					i,
					idx1,
					i,
					idx2,
					i,
					idx2
				);
				break;
			}
			case OP::NEQ: {
				auto idx1 = sql_param_idx;
				sql_param_idx++;
				auto idx2 = sql_param_idx;
				sql_param_idx++;
				tpl_stream << fmt::format(
					"INNER JOIN food_nutrient AS fn{} "
					" ON fn{}.food_id = f.id "
					" AND fn{}.nutrient_id = :{} "
					" AND (fn{}.value < (:{} - 0.001) or fn{}.value > (:{} + 0.001)) ",
					i,
					i,
					i,
					idx1,
					i,
					idx2,
					i,
					idx2
				);
				break;
			}
			case OP::BETWEEN: {
				auto idx1 = sql_param_idx;
				sql_param_idx++;
				auto idx2 = sql_param_idx;
				sql_param_idx++;
				auto idx3 = sql_param_idx;
				sql_param_idx++;

				tpl_stream << fmt::format(
					" INNER JOIN food_nutrient AS fn{} "
					" ON fn{}.food_id = f.id "
					" AND fn{}.nutrient_id = :{} "
					" AND (fn{}.value >= :{} AND fn{}.value <= :{}) ",
					i,
					i,
					i,
					idx1,
					i,
					idx2,
					i,
					idx3
				);
				break;
			}
		}
	}

	if (q.title.size() > 0)
	{
		tpl_stream << " WHERE ";

		tpl_stream << fmt::format(
			" fl.title LIKE :{} ",
			sql_param_idx
		);
		sql_param_idx++;
	}

	return tpl_stream.str();
}

static void bind_params(
	sqlw::Statement& stmt,
	const wholth::FoodsQuery& q
)
{
	using OP = wholth::nutrient_filter::Operation;

	size_t idx = 1;

	if (q.locale_id.size() > 0)
	{
		stmt.bind(idx, q.locale_id, sqlw::Type::SQL_INT);
		idx++;
	}

	for (const wholth::nutrient_filter::Entry& entry : q.nutrient_filters)
	{
		if (is_empty(entry))
		{
			break;
		}

		const auto& value = std::get<wholth::nutrient_filter::Value::index>(entry);

		switch (std::get<OP>(entry))
		{
			case OP::EQ: {
				stmt.bind(
					idx,
					std::get<wholth::nutrient_filter::NutrientId::index>(entry),
					sqlw::Type::SQL_INT
				);
				idx++;
				stmt.bind(
					idx,
					value,
					sqlw::Type::SQL_DOUBLE
				);
				idx++;
				break;
			}
			case OP::NEQ: {
				stmt.bind(
					idx,
					std::get<wholth::nutrient_filter::NutrientId::index>(entry),
					sqlw::Type::SQL_INT
				);
				idx++;
				stmt.bind(
					idx,
					value,
					sqlw::Type::SQL_DOUBLE
				);
				idx++;
				break;
			}
			case OP::BETWEEN: {
				std::array<std::string_view, 2> values {
					"0",
					"0"
				};

				for (size_t i = 0; i < value.length(); i++) {
					if (',' == value[i] && (value.length() - 1) != i)
					{
						values[0] = value.substr(0, i);
						values[1] = value.substr(i + 1);
						break;
					}
				}

				stmt.bind(
					idx,
					std::get<wholth::nutrient_filter::NutrientId::index>(entry),
					sqlw::Type::SQL_INT
				);
				idx++;
				stmt.bind(
					idx,
					values[0],
					sqlw::Type::SQL_DOUBLE
				);
				idx++;
				stmt.bind(
					idx,
					values[1],
					sqlw::Type::SQL_DOUBLE
				);
				idx++;
				break;
			}
		}
	}

	if (q.title.size() > 0)
	{
		std::string _title = fmt::format("%{}%", q.title);

		stmt.bind(
			idx,
			_title,
			sqlw::Type::SQL_TEXT
		);
		idx++;
	}
}

static std::string create_entity_query_sql(
	const wholth::FoodsQuery& q,
	size_t limit
)
{
	return fmt::format(
		"SELECT "
			"f.id, "
			"COALESCE(fl.title, '[N/A]') AS title, "
			"COALESCE("
				"seconds_to_readable_time(rs.seconds),"
				"'[N/A]'"
			") AS time "
		"FROM food f "
		" {1} "
		" {2} "
		"GROUP BY f.id "
		"ORDER BY f.id ASC, fl.locale_id ASC, fl.title ASC "
		"LIMIT {3} "
		"OFFSET {4}",
		q.title,
		create_joins(q.locale_id),
		create_where(q),
		limit,
		limit * q.page
	);
}

static std::string create_pagination_query_sql(
	const wholth::FoodsQuery& q,
	size_t limit
)
{
	return fmt::format(
		"SELECT "
			"r.cnt AS cnt, "
			"MAX(1, CAST(ROUND(CAST(r.cnt AS float) / {0} + 0.49) as int)) AS max_page, "
			"'{1}' || '/' || (MAX(1, CAST(ROUND(CAST(r.cnt AS float) / {2} + 0.49) AS int))) AS paginator_str "
		"FROM ( "
			"SELECT "
				"COUNT(g.id) AS cnt "
			"FROM ( "
				"SELECT f.id AS id "
				"FROM food f "
				" {3} "
				" {4} "
				"GROUP BY f.id "
				"ORDER BY f.id ASC, fl.locale_id ASC, fl.title ASC "
			") AS g "
		") AS r",
		limit,
		q.page + 1,
		limit,
		create_joins(q.locale_id),
		create_where(q)
	);
}

auto wholth::list_foods(
	std::span<wholth::entity::shortened::Food> span,
	std::string& buffer,
	PaginationInfo& p_info,
	const FoodsQuery& q,
	sqlw::Connection* con
) noexcept -> SC
{
	sqlw::Statement entity_stmt {con};
	sqlw::Statement paginator_stmt {con};
	// @todo: rethink this jank!
	constexpr size_t field_count = 3;

	auto entity_sql = create_entity_query_sql(q, span.size());
	auto paginator_sql = create_pagination_query_sql(q, span.size());
	
	std::stringstream buffer_stream;

	wholth::utils::LengthContainer itr {
		(span.size() * field_count) + 3
	};

	paginator_stmt.prepare(paginator_sql);

	bind_params(paginator_stmt, q);

	paginator_stmt([&buffer_stream, &itr](sqlw::Statement::ExecArgs e)
		{
			buffer_stream << e.column_value;

			itr.add(e.column_value.size());
		}
	);

	entity_stmt.prepare(entity_sql);

	bind_params(entity_stmt, q);

	entity_stmt([&buffer_stream, &itr](sqlw::Statement::ExecArgs e)
		{
			buffer_stream << e.column_value;

			itr.add(e.column_value.size());
		}
	);

	if (!(buffer_stream.rdbuf()->in_avail() > 0))
	{
		// todo test
		return SC::ENTITY_NOT_FOUND;
	}

	buffer = buffer_stream.str();

	p_info.element_count = itr.next(buffer);
	p_info.max_page = itr.next(buffer);
	p_info.progress_string = itr.next(buffer);

	size_t j = 0;
	for (
		std::string_view id = itr.next(buffer);
		id != wholth::utils::NIL;
		id = itr.next(buffer)
	)
	{
		wholth::entity::shortened::Food entry;

		entry.id = id;
		entry.title = itr.next(buffer);
		entry.preparation_time = itr.next(buffer);

		span[j] = entry;
		j++;
	}

	return SC::NO_ERROR;
}

wholth::StatusCode wholth::insert_food(
	const wholth::entity::editable::Food& food,
	std::string& result_id,
	sqlw::Connection& con,
	wholth::entity::locale::id_t locale_id
) noexcept
{
	if (locale_id.size() == 0 || !sqlw::utils::is_numeric(locale_id))
	{
		return SC::INVALID_LOCALE_ID;
	}

	using sqlw::status::is_ok;

	sqlw::Statement stmt {&con};
	stmt("SAVEPOINT insert_food_pnt");

	const std::string now = wholth::utils::current_time_and_date();

	stmt.prepare(
			"INSERT INTO food (created_at) VALUES (:1); "
		)
		.bind(1, now)
		.exec();

	if (!sqlw::status::is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return SC::SQL_STATEMENT_ERROR;
	}

	stmt(
		"SELECT last_insert_rowid()",
		[&result_id](sqlw::Statement::ExecArgs e) {
			result_id = e.column_value;
		}
	);

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return SC::SQL_STATEMENT_ERROR;
	}

	stmt.prepare(
			"INSERT INTO food_localisation (food_id,locale_id,title,description) "
			"VALUES (:1,:2,lower(trim(:3)),:4)"
		)
		.bind(1, result_id)
		.bind(2, locale_id)
		.bind(3, food.title)
		.bind(4, food.description)
		.exec();

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return SC::SQL_STATEMENT_ERROR;
	}

	stmt("RELEASE insert_food_pnt");

	return SC::NO_ERROR;
}

wholth::UpdateFoodStatus wholth::update_food(
	const wholth::entity::editable::Food& food,
	sqlw::Connection& con,
	wholth::entity::locale::id_t locale_id
) noexcept
{
	using sqlw::status::is_ok;
	wholth::UpdateFoodStatus status {};

	if (!(food.title.size() > 0)
		|| food.title.size() == count_spaces(food.title)
	) {
		status.title = SC::EMPTY_FOOD_TITLE;
	}
	else if (food.title == wholth::utils::NIL) {
		status.title = SC::UNCHANGED_FOOD_TITLE;
	}

	if (food.description == wholth::utils::NIL) {
		status.description = SC::UNCHANGED_FOOD_DESCRIPTION;
	}

	if (!status.title && !status.description) {
		return status;
	}

	sqlw::Statement stmt {&con};
	stmt("SAVEPOINT update_food_pnt");

	std::stringstream ss;
	ss << "UPDATE food_localisation SET ";

	size_t idx = 1;

	if (!!status.title) {
		ss << fmt::format(
			"title = lower(trim(:{}))",
			idx
		);
		idx++;

		if (!!status.description) {
			ss << ",";
		}
	}

	if (!!status.description) {
		ss << fmt::format("description = :{} ", idx);
		idx++;
	}

	ss << fmt::format(
		"WHERE food_id = :{} AND locale_id = :{}",
		idx,
		idx + 1
	);

	stmt.prepare(ss.str());

	idx = 1;

	if (!!status.title) {
		stmt.bind(idx, food.title, sqlw::Type::SQL_TEXT);
		idx++;
	}

	if (!!status.description) {
		stmt.bind(idx, food.description, sqlw::Type::SQL_TEXT);
		idx++;
	}

	stmt.bind(idx, food.id, sqlw::Type::SQL_INT);
	stmt.bind(idx + 1, locale_id, sqlw::Type::SQL_INT);

	stmt.exec();

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO update_food_pnt");
		status.rc = SC::SQL_STATEMENT_ERROR;
	}

	stmt("RELEASE update_food_pnt");

	return status;
}

SC wholth::remove_food(wholth::entity::food::id_t food_id, sqlw::Connection& con) noexcept
{
	using sqlw::status::is_ok;

	if (food_id.size() == 0 || !sqlw::utils::is_numeric(food_id))
	{
		return SC::INVALID_FOOD_ID;
	}

	sqlw::Statement stmt {&con};

	stmt("SAVEPOINT remove_food_pnt");

	stmt
		.prepare("DELETE FROM food WHERE id = :1")
		.bind(1, food_id, sqlw::Type::SQL_INT)
		.exec();

	SC rc = check_stmt(stmt);

	if (!rc)
	{
		stmt("ROLLBACK TO remove_food_pnt");
	}
	else
	{
		stmt("RELEASE remove_food_pnt");
	}

	return rc;
}

void wholth::add_nutrients(
	wholth::entity::food::id_t food_id,
	const std::span<const wholth::entity::editable::food::Nutrient> nutrients,
	sqlw::Connection& con
) noexcept
{
	using sqlw::status::is_ok;

	sqlw::Statement stmt {&con};
	stmt("SAVEPOINT add_food_nutrient_pnt");

	if (!(nutrients.size() > 0))
	{
		return;
	}

	wholth::utils::IndexSequence<3, 1> idxs{};
	std::stringstream ss;

	ss <<
		"INSERT INTO food_nutrient (food_id,nutrient_id,value) "
		"VALUES ";

	for (const auto& nutrient : nutrients) {
		if (nutrient.value.length() == 0) {
			break;
		}

		ss << fmt::format(
			"(:{},:{},:{}),",
			idxs[0],
			idxs[1],
			idxs[2]
		);

		idxs.advance();
	}

	std::string sql = ss.str();
	
	stmt.prepare(sql.substr(0, sql.size() - 1));

	idxs.reset();

	for (const auto& nutrient : nutrients) {

		if (nutrient.value.length() == 0) {
			break;
		}

		stmt.bind(idxs[0], food_id, sqlw::Type::SQL_INT);
		stmt.bind(idxs[1], nutrient.id, sqlw::Type::SQL_INT);

		try {
			stmt.bind(idxs[2], nutrient.value, sqlw::Type::SQL_DOUBLE);
		} catch (const std::invalid_argument& e) {
			break;
		};

		idxs.advance();
	}

	stmt.exec();

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO add_food_nutrient_pnt");
		return;
	}

	stmt("RELEASE add_food_nutrient_pnt");
}

void wholth::remove_nutrients(
	wholth::entity::food::id_t food_id,
	const std::span<const wholth::entity::editable::food::Nutrient> nutrients,
	sqlw::Connection& con
) noexcept
{
	using sqlw::status::is_ok;

	const std::string now = wholth::utils::current_time_and_date();
	sqlw::Statement stmt {&con};

	if (!(nutrients.size() > 0))
	{
		return;
	}

	for (const auto& nutrient : nutrients) {
		stmt("SAVEPOINT remove_food_nutrient_pnt");

		stmt.prepare("DELETE FROM food_nutrient WHERE food_id = :1 AND nutrient_id = :2")
			.bind(1, food_id, sqlw::Type::SQL_INT)
			.bind(2, nutrient.id, sqlw::Type::SQL_INT)
			.exec();

		if (!is_ok(stmt.status()))
		{
			stmt("ROLLBACK TO remove_food_nutrient_pnt");
			return;
		}

		stmt("RELEASE remove_food_nutrient_pnt");
	}
}

// @todo - подумать как показывать ошибки.
void wholth::update_nutrients(
	wholth::entity::food::id_t food_id,
	const std::span<const wholth::entity::editable::food::Nutrient> nutrients,
	sqlw::Connection& con
) noexcept
{
	using sqlw::status::is_ok;

	if (!(nutrients.size() > 0))
	{
		return;
	}

	const std::string now = wholth::utils::current_time_and_date();
	sqlw::Statement stmt {&con};

	for (const auto& nutrient : nutrients)
	{
		stmt("SAVEPOINT update_food_nutrient_pnt");

		stmt.prepare(
			"UPDATE food_nutrient SET value = :1 "
			"WHERE nutrient_id = :2 AND food_id = :3"
		)
			.bind(1, nutrient.value)
			.bind(2, nutrient.id)
			.bind(3, food_id)
			.exec();

		if (!is_ok(stmt.status()))
		{
			stmt("ROLLBACK TO update_food_nutrient_pnt");
			continue;
		}

		stmt("RELEASE update_food_nutrient_pnt");
	}
}

void wholth::add_steps(
	wholth::entity::food::id_t food_id,
	const std::span<const wholth::entity::editable::food::RecipeStep> steps,
	sqlw::Connection& con,
	wholth::entity::locale::id_t locale_id
) noexcept
{
	using sqlw::status::is_ok;

	if (!(steps.size() > 0))
	{
		return;
	}

	sqlw::Statement stmt {&con};
	stmt("SAVEPOINT add_steps_pnt");

	stmt
		.prepare("INSERT INTO recipe_step (recipe_id,seconds) VALUES (:1,:2)")
		.bind(1, food_id)
		.bind(2, steps[0].seconds)
		.exec();

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO add_steps_pnt");
		return;
	}

	std::string recipe_step_id;

	stmt(
		"SELECT last_insert_rowid()",
		[&recipe_step_id](sqlw::Statement::ExecArgs e) {
			recipe_step_id = e.column_value;
		}
	);

	stmt
		.prepare(
			"INSERT INTO recipe_step_localisation (recipe_step_id,locale_id,description) "
			"VALUES (:1,:2,:3) "
		)
		.bind(1, recipe_step_id)
		.bind(2, locale_id)
		.bind(3, steps[0].description)
		.exec();

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO add_steps_pnt");
		return;
	}

	stmt("RELEASE add_steps_pnt");
}

void wholth::remove_steps(
	const std::span<const wholth::entity::editable::food::RecipeStep> steps,
	sqlw::Connection& con
) noexcept
{
	using sqlw::status::is_ok;

	if (!(steps.size() > 0))
	{
		return;
	}

	sqlw::Statement stmt {&con};

	// @todo: maybe redo?
	for (const auto& step : steps)
	{
		stmt("SAVEPOINT remove_steps_pnt");

		stmt.prepare("DELETE FROM recipe_step WHERE id = :1")
			.bind(1, step.id, sqlw::Type::SQL_INT)
			.exec();

		if (!is_ok(stmt.status()))
		{
			stmt("ROLLBACK TO remove_steps_pnt");
			return;
		}

		stmt("RELEASE remove_steps_pnt");
	}
}

void update_steps(
	const std::span<const wholth::entity::editable::food::RecipeStep> steps,
	sqlw::Connection& con
)
{
	using sqlw::status::is_ok;
	using wholth::utils::NIL;

	if (!(steps.size() > 0))
	{
		return;
	}

	sqlw::Statement stmt {&con};
	std::stringstream ss;

	for (const auto& step : steps)
	{
		ss.clear();

		stmt("SAVEPOINT update_steps_pnt");
		bool _description = step.description != NIL;
		bool _time = step.seconds != NIL;

		stmt.prepare(fmt::format(
			"UPDATE recipe_step_localisation SET description = {}, seconds = {}",
			_description ? ":1" : "description",
			_time ? ":2" : "seconds"
		));

		if (_description) {
			stmt.bind(1, step.description, sqlw::Type::SQL_TEXT);
		}
		if (_time) {
			stmt.bind(2, step.seconds, sqlw::Type::SQL_INT);
		}

		stmt.exec();

		if (!is_ok(stmt.status()))
		{
			stmt("ROLLBACK TO update_steps_pnt");
			continue;
		}

		stmt("RELEASE update_steps_pnt");
	}
}

void wholth::add_ingredients(
	wholth::entity::recipe_step::id_t recipe_step_id,
	const std::span<const wholth::entity::editable::food::Ingredient> foods,
	sqlw::Connection& con
) noexcept
{
	using sqlw::status::is_ok;

	sqlw::Statement stmt {&con};
	stmt("SAVEPOINT add_foods_to_step_pnt");

	if (!(foods.size() > 0))
	{
		return;
	}

	std::stringstream ss;
	wholth::utils::IndexSequence<3, 1> idxs;

	ss << "INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) "
		"VALUES ";

	for (const auto& step_food : foods)
	{
		if (step_food.food_id.length() == 0) {
			break;
		}

		ss << fmt::format(
			"(:{},:{},:{}),",
			idxs[0],
			idxs[1],
			idxs[2]
		);

		idxs.advance();
	}

	std::string sql = ss.str();
	
	stmt.prepare(sql.substr(0, sql.size() - 1));

	idxs.reset();

	for (const auto& step_food : foods)
	{
		if (step_food.food_id.length() == 0) {
			break;
		}

		stmt.bind(idxs[0], recipe_step_id, sqlw::Type::SQL_INT);
		stmt.bind(idxs[1], step_food.food_id, sqlw::Type::SQL_INT);
		stmt.bind(idxs[2], step_food.canonical_mass, sqlw::Type::SQL_DOUBLE);

		idxs.advance();
	}

	stmt.exec();
}

void wholth::update_ingredients(
	wholth::entity::recipe_step::id_t recipe_step_id,
	const std::span<const wholth::entity::editable::food::Ingredient> foods,
	sqlw::Connection& con
) noexcept
{
	using sqlw::status::is_ok;

	sqlw::Statement stmt {&con};

	if (!(foods.size() > 0))
	{
		return;
	}

	for (const auto& ing : foods)
	{
		stmt("SAVEPOINT update_ingreddients_pnt");
		stmt.prepare(
			"UPDATE recipe_step_food SET canonical_mass = :1 "
			"WHERE recipe_step_id = :2 AND food_id = :3"
		)
			.bind(1, ing.canonical_mass, sqlw::Type::SQL_DOUBLE)
			.bind(2, recipe_step_id, sqlw::Type::SQL_INT)
			.bind(3, ing.food_id, sqlw::Type::SQL_INT)
			.exec();
		
		if (!is_ok(stmt.status())) {
			stmt("ROLLBACK TO update_ingreddients_pnt");
		}
		else {
			stmt("RELEASE update_ingreddients_pnt");
		}
	}
}

void wholth::remove_ingredients(
	wholth::entity::recipe_step::id_t recipe_step_id,
	const std::span<const wholth::entity::editable::food::Ingredient> foods,
	sqlw::Connection& con
) noexcept
{
	using sqlw::status::is_ok;

	if (!(foods.size() > 0))
	{
		return;
	}

	sqlw::Statement stmt {&con};

	// todo: maybe redo?
	for (size_t i = 0; i < foods.size(); i++)
	{
		stmt("SAVEPOINT remove_ingreddients_pnt");

		stmt.prepare(
			"DELETE FROM recipe_step_food "
			"WHERE recipe_step_id = :1 AND food_id = :2")
			.bind(1, recipe_step_id, sqlw::Type::SQL_INT)
			.bind(2, foods[i].food_id, sqlw::Type::SQL_INT)
			.exec();

		if (!is_ok(stmt.status())) {
			stmt("ROLLBACK TO remove_ingreddients_pnt");
		}
		else {
			stmt("RELEASE remove_ingreddients_pnt");
		}
	}
}

SC wholth::expand_food(
	wholth::entity::expanded::Food& food,
	std::string &buffer,
	wholth::entity::food::id_t id,
	wholth::entity::locale::id_t locale_id,
	sqlw::Connection* con
) noexcept
{
	if (locale_id.size() == 0 || !sqlw::utils::is_numeric(locale_id))
	{
		return SC::INVALID_LOCALE_ID;
	}

	sqlw::Statement stmt {con};

	std::stringstream ss {};
	wholth::utils::LengthContainer itr {
		5 // id, title, description, calories, prep_time
	};

	stmt
		.prepare(
			"SELECT "
				"f.id AS id, "
				"COALESCE(fl.title, '[N/A]') AS title, "
				"COALESCE(fl.description, '[N/A]') AS description, "
				"COALESCE("
					"seconds_to_readable_time(rs.seconds),"
					"'[N/A]'"
				") AS preparation_time "
			"FROM food f "
			"LEFT JOIN food_localisation fl "
				"ON fl.food_id = f.id AND fl.locale_id = :1 "
			"LEFT JOIN recipe_step rs "
				"ON rs.recipe_id = f.id "
			"WHERE f.id = :2"
		)
		.bind(1, locale_id, sqlw::Type::SQL_INT)
		.bind(2, id, sqlw::Type::SQL_INT)
		.exec([&ss, &itr](sqlw::Statement::ExecArgs e) {
			ss << e.column_value;
			itr.add(e.column_value.size());
		});

	SC rc = check_stmt(stmt);

	if (!rc) {
		return rc;
	}

	if (ss.rdbuf()->in_avail() == 0) {
		return SC::ENTITY_NOT_FOUND;
	}

	buffer = ss.str();

	food.id = itr.next(buffer);
	food.title = itr.next(buffer);
	food.description = itr.next(buffer);
	food.preparation_time = itr.next(buffer);

	return rc;
}

auto wholth::list_steps(
	std::span<wholth::entity::expanded::food::RecipeStep> steps,
	std::string& buffer,
	wholth::entity::food::id_t food_id,
	wholth::entity::locale::id_t locale_id,
	sqlw::Connection* con
) noexcept -> StatusCode
{
	if (!(locale_id.size() >= 1) || !sqlw::utils::is_numeric(locale_id)) {
		return SC::INVALID_LOCALE_ID;
	}

	sqlw::Statement stmt {con};

	std::stringstream ss {};
	wholth::utils::LengthContainer itr {
		3 // id, time, description
		* steps.size()
	};
	stmt
		.prepare(
			"SELECT "
				"rs.id AS id,"
				"COALESCE("
					"seconds_to_readable_time(rs.seconds),"
					"'[N/A]'"
				") AS time, "
				"COALESCE(rsl.description, '[N/A]') AS description "
			"FROM recipe_step rs "
			"LEFT JOIN recipe_step_localisation rsl "
				"ON rsl.recipe_step_id = rs.id AND locale_id = :1 "
			"WHERE rs.recipe_id = :2 "
			"ORDER BY rs.id ASC "
			"LIMIT :3 "
		)
		.bind(1, locale_id, sqlw::Type::SQL_INT)
		.bind(2, food_id, sqlw::Type::SQL_INT)
		// todo: check that size is not bigger than int.
		.bind(3, static_cast<int>(steps.size()));

	stmt([&ss, &itr](sqlw::Statement::ExecArgs e) {
		ss << e.column_value;
		itr.add(e.column_value.size());
	});

	if (!sqlw::status::is_ok(stmt.status())) {
		return SC::SQL_STATEMENT_ERROR;
	}

	if (ss.rdbuf()->in_avail() == 0) {
		return SC::ENTITY_NOT_FOUND;
	}

	buffer = ss.str();

	for (size_t j = 0; j < steps.size(); j++)
	{
		auto& entry = steps[j];

		entry.id = itr.next(buffer);
		entry.time = itr.next(buffer);
		entry.description = itr.next(buffer);
	}

	return SC::NO_ERROR;
}

auto wholth::list_ingredients(
	std::span<wholth::entity::expanded::food::Ingredient> ingredients,
	std::string& buffer,
	wholth::entity::food::id_t recipe_id,
	wholth::entity::locale::id_t locale_id,
	sqlw::Connection* con
) noexcept -> SC
{
	if (!(locale_id.size() >= 1) || !sqlw::utils::is_numeric(locale_id)) {
		return SC::INVALID_LOCALE_ID;
	}

	sqlw::Statement stmt {con};

	std::stringstream ss {};
	wholth::utils::LengthContainer itr {
		4 // food_id, title, canonica_mass, ingredient_count
		* ingredients.size()
	};
	stmt
		.prepare(
			"SELECT "
				"f.id AS food_id, "
				"COALESCE(fl.title, '[N/A]') AS title, "
				"rsf.canonical_mass AS canonical_mass, "
				"( "
					"SELECT COUNT(rs1.id) "
					"FROM recipe_step rs1 "
					"WHERE rs1.recipe_id = f.id "
				") AS ingredient_count "
			"FROM recipe_step rs "
			"LEFT JOIN recipe_step_food rsf "
				"ON rsf.recipe_step_id = rs.id "
			"LEFT JOIN food f "
				"ON f.id = rsf.food_id "
			"LEFT JOIN food_localisation fl "
				"ON fl.food_id = f.id AND fl.locale_id = :1 "
			"WHERE rs.recipe_id = :2 "
			"LIMIT :3 "
		)
		.bind(1, locale_id, sqlw::Type::SQL_INT)
		.bind(2, recipe_id, sqlw::Type::SQL_INT)
		// todo check size
		.bind(3, static_cast<int>(ingredients.size()));

	stmt([&ss, &itr](sqlw::Statement::ExecArgs e) {
		ss << e.column_value;
		itr.add(e.column_value.size());
	});

	if (!sqlw::status::is_ok(stmt.status())) {
		return SC::SQL_STATEMENT_ERROR;
	}

	if (ss.rdbuf()->in_avail() == 0) {
		return SC::ENTITY_NOT_FOUND;
	}

	buffer = ss.str();

	for (size_t j = 0; j < ingredients.size(); j++)
	{
		auto& entry = ingredients[j];

		entry.food_id = itr.next(buffer);
		entry.title = itr.next(buffer);
		entry.canonical_mass = itr.next(buffer);
		entry.ingredient_count = itr.next(buffer);
	}

	return SC::NO_ERROR;
}

auto wholth::list_nutrients(
	std::span<wholth::entity::expanded::food::Nutrient> nutrients,
	std::string& buffer,
	wholth::entity::food::id_t food_id,
	wholth::entity::locale::id_t locale_id,
	sqlw::Connection* con
) noexcept -> SC
{
	if (!(locale_id.size() >= 1) || !sqlw::utils::is_numeric(locale_id)) {
		return SC::INVALID_LOCALE_ID;
	}

	sqlw::Statement stmt {con};

	std::stringstream ss {};
	wholth::utils::LengthContainer itr {
		5 // id, title, value, unit, user_value
		* nutrients.size()
	};
	stmt
		.prepare(
			"WITH RECURSIVE "
			"recipe_tree( "
				"lvl, "
				"recipe_id, "
				"recipe_mass, "
				"recipe_ingredient_count, "
				"ingredient_id, "
				"ingredient_mass, "
				"ingredient_weight "
			") AS ( "
				"SELECT * FROM recipe_tree_node root "
				"WHERE root.recipe_id = :1 "
				"UNION "
				"SELECT "
					"rt.lvl + 1, "
					"node.recipe_id, "
					"node.recipe_mass, "
					"node.recipe_ingredient_count, "
					"node.ingredient_id, "
					"node.ingredient_mass, "
					"node.ingredient_mass / node.recipe_mass * rt.ingredient_weight "
				"FROM recipe_tree rt "
				"INNER JOIN recipe_tree_node node "
					"ON node.recipe_id = rt.ingredient_id "
				"ORDER BY 1 DESC "
			") "
			"SELECT "
				"id, title, value, unit, user_value "
			"FROM ( "
				"SELECT "
					"fn.nutrient_id AS id, "
					"SUM(rt.ingredient_weight) AS sum_weight, "
					"COALESCE(nl.title, '[N/A]') AS title, "
					"n.unit AS unit, "
					"SUM(fn.value * rt.ingredient_weight) * 100 / root_recipe_info.recipe_mass AS value, "
					"root_food_nutrient.value AS user_value "
				"FROM recipe_tree rt "
				"LEFT JOIN recipe_info ingredient_info "
					"ON ingredient_info.recipe_id = rt.ingredient_id "
				"INNER JOIN food_nutrient fn "
					"ON fn.food_id = rt.ingredient_id "
				"LEFT JOIN nutrient n "
					"ON n.id = fn.nutrient_id "
				"LEFT JOIN nutrient_localisation nl "
					"ON nl.nutrient_id = n.id AND nl.locale_id = :2 "
				"LEFT JOIN recipe_info root_recipe_info "
					"ON root_recipe_info.recipe_id = :1 "
				"LEFT JOIN food_nutrient root_food_nutrient "
					"ON root_food_nutrient.food_id = :1 AND root_food_nutrient.nutrient_id = n.id "
				// So that ingredients that are also recipes will not have their
				// user specified nutrient values summated.
				"WHERE ingredient_info.recipe_id IS NULL "
				"GROUP BY fn.nutrient_id "
				"HAVING sum_weight = 1 "
				"ORDER BY n.position ASC "
				"LIMIT :3 "
			") "
		)
		.bind(1, food_id, sqlw::Type::SQL_INT)
		.bind(2, locale_id, sqlw::Type::SQL_INT)
		// todo check size
		.bind(3, static_cast<int>(nutrients.size()));


	/* size_t i = 0; */
	stmt([&ss, &itr/*, &i */](sqlw::Statement::ExecArgs e) {
		/* if (0 == i % e.column_count) { */
		/* 	fmt::print("\n"); */
		/* } */
		/* i++; */
		/* fmt::print("{}: {}\n", e.column_name, e.column_value); */

		ss << e.column_value;
		itr.add(e.column_value.size());
	});

	/* return SC{}; */

	if (!sqlw::status::is_ok(stmt.status())) {
		return SC::SQL_STATEMENT_ERROR;
	}

	if (ss.rdbuf()->in_avail() == 0) {
		return SC::ENTITY_NOT_FOUND;
	}

	buffer = ss.str();

	for (size_t j = 0; j < nutrients.size(); j++)
	{
		auto& entry = nutrients[j];

		entry.id = itr.next(buffer);
		entry.title = itr.next(buffer);
		entry.value = itr.next(buffer);
		entry.unit = itr.next(buffer);
		entry.user_value = itr.next(buffer);
	}

	return SC::NO_ERROR;
}

SC wholth::describe_error(
	SC ec,
	std::string& buffer,
	wholth::entity::locale::id_t locale_id,
	sqlw::Connection* con
) noexcept
{
	sqlw::Statement stmt {con};
	// @todo: think about caching.
	stmt
		.prepare(fmt::format(
			"SELECT el.description "
			"FROM error_localisation el "
			"WHERE el.error_id = {} AND locale_id = :1",
			static_cast<int>(ec)
		))
		.bind(1, locale_id, sqlw::Type::SQL_INT)
		.exec([&buffer](sqlw::Statement::ExecArgs e) {
			buffer = e.column_value;
		});

	return check_stmt(stmt);
}

std::ostream& operator<<(std::ostream& out, const wholth::entity::expanded::Food& f)
{
	out << "{\n id: " << f.id
		<< "\n title: " << f.title
		<< "\n descriotion: " << f.description
		<< "\n preparation_time: " << f.preparation_time
		<< "\n}";
	return out;
}

std::string_view wholth::view(wholth::StatusCode rc)
{
	switch (rc) {
		case wholth::StatusCode::SQL_STATEMENT_ERROR: return "SQL_STATEMENT_ERROR";
		case wholth::StatusCode::NO_ERROR: return "NO_ERROR";
		case wholth::StatusCode::ENTITY_NOT_FOUND: return "ENTITY_NOT_FOUND";
		case wholth::StatusCode::INVALID_LOCALE_ID: return "INVALID_LOCALE_ID";
		case wholth::StatusCode::INVALID_FOOD_ID: return "INVALID_FOOD_ID";
		case wholth::StatusCode::EMPTY_FOOD_TITLE: return "EMPTY_FOOD_TITLE";
		case wholth::StatusCode::UNCHANGED_FOOD_TITLE: return "UNCHANGED_FOOD_TITLE";
		case wholth::StatusCode::UNCHANGED_FOOD_DESCRIPTION: return "UNCHANGED_FOOD_DESCRIPTION";
	}
}
