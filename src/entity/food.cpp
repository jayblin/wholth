#include "wholth/entity/food.hpp"
#include "fmt/color.h"
#include "fmt/core.h"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "wholth/concepts.hpp"
#include "wholth/entity/nutrient.hpp"
#include "wholth/pager.hpp"
#include "wholth/utils.hpp"
#include <charconv>
#include <exception>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

/* wholth::entity::food::Input wholth::entity::food::input::initialize() */
/* { */
/* 	return {{"##food_title"}, {"##food_calories"}}; */
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

static std::string_view create_joins(const wholth::FoodsQuery& q)
{
	/* if (q.locale_id.npos == q.locale_id.find(",")) */
	/* { */
	/* 	return "LEFT JOIN food_localisation fl " */
	/* 	   "ON fl.food_id = f.id AND fl.locale_id IN (?1) "; */
	/* } */

	if (q.locale_id.size() == 0)
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
				"CASE WHEN rs.seconds IS NULL THEN rs.seconds "
				"WHEN rs.seconds < 60 THEN ((rs.seconds) || 's') "
				"WHEN rs.seconds < 3600 THEN ((rs.seconds / 60) || 'm') "
				"ELSE ((rs.seconds / 3600) || 'h') "
				"END "
				", '[N/A]') as time "
		"FROM food f "
		" {1} "
		" {2} "
		"GROUP BY f.id "
		"ORDER BY f.id ASC, fl.locale_id ASC, fl.title ASC "
		"LIMIT {3} "
		"OFFSET {4}",
		q.title,
		create_joins(q),
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
		create_joins(q),
		create_where(q)
	);
}

template<> template<>
PaginationInfo wholth::Pager<wholth::entity::viewable::Food>::query_page<wholth::FoodsQuery>(
	std::span<wholth::entity::viewable::Food> span,
	sqlw::Connection* con,
	const FoodsQuery& q
)
{
	PaginationInfo p_info {};

	size_t buffer_idx = (m_buffer_idx + 1) % 2;

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

	auto s = buffer_stream.str();

	m_buffers[buffer_idx] = std::move(s);

	auto& buffer_ref = m_buffers[buffer_idx];

	if (!(buffer_ref.size() > 0))
	{
		return p_info;
	}

	p_info.element_count = itr.next(buffer_ref);
	p_info.max_page = itr.next(buffer_ref);
	p_info.progress_string = itr.next(buffer_ref);

	size_t j = 0;
	for (std::string_view id = itr.next(buffer_ref); id != wholth::utils::NIL; id = itr.next(buffer_ref))
	{
		wholth::entity::viewable::Food entry;

		entry.id = id;
		entry.title = itr.next(buffer_ref);
		entry.preparation_time = itr.next(buffer_ref);

		span[j] = entry;
		j++;
	}

	m_buffer_idx = buffer_idx;

	return p_info;
}

std::optional<wholth::entity::food::id_t> wholth::insert_food(
	const wholth::entity::editable::Food& food,
	sqlw::Connection& con,
	wholth::entity::locale::id_t locale_id
)
{
	using sqlw::status::is_ok;

	sqlw::Statement stmt {&con};
	stmt("SAVEPOINT insert_food_pnt");

	const std::string now = wholth::utils::current_time_and_date();
	std::string food_id;

	stmt.prepare(
			"INSERT INTO food (created_at) VALUES (:1); "
		)
		.bind(1, now)
		.exec();

	if (!sqlw::status::is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return std::nullopt;
	}

	stmt(
		"SELECT last_insert_rowid()",
		[&food_id](sqlw::Statement::ExecArgs e) {
			food_id = e.column_value;
		}
	);

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return std::nullopt;
	}

	stmt.prepare(
			"INSERT INTO food_localisation (food_id,locale_id,title,description) "
			"VALUES (:1,:2,lower(trim(:3)),:4)"
		)
		.bind(1, food_id)
		.bind(2, locale_id)
		.bind(3, food.title)
		.bind(4, food.description)
		.exec();

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return std::nullopt;
	}

	stmt("RELEASE insert_food_pnt");

	return food_id;
}

void wholth::update_food(
	const wholth::entity::editable::Food& food,
	sqlw::Connection& con,
	 wholth::entity::locale::id_t locale_id
)
{
	using sqlw::status::is_ok;

	bool is_title = food.title.size() > 0
		&& food.title.size() != count_spaces(food.title)
		&& food.title != wholth::utils::NIL;
	bool is_description = food.description != wholth::utils::NIL;

	if (!is_title && !is_description) {
		return;
	}

	sqlw::Statement stmt {&con};
	stmt("SAVEPOINT update_food_pnt");

	std::stringstream ss;
	ss << "UPDATE food_localisation SET ";

	size_t idx = 1;

	if (is_title) {
		ss << fmt::format(
			"title = lower(trim(:{}))",
			idx
		);
		idx++;

		if (is_description) {
			ss << ",";
		}
	}

	if (is_description) {
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

	if (is_title) {
		stmt.bind(idx, food.title, sqlw::Type::SQL_TEXT);
		idx++;
	}

	if (is_description) {
		stmt.bind(idx, food.description, sqlw::Type::SQL_TEXT);
		idx++;
	}

	try {
		stmt.bind(idx, food.id, sqlw::Type::SQL_INT);
		stmt.bind(idx + 1, locale_id, sqlw::Type::SQL_INT);
	}
	catch (const std::exception& e) {
		stmt("ROLLBACK TO update_food_pnt");
		return;
	}

	stmt.exec();

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO update_food_pnt");
	}

	stmt("RELEASE update_food_pnt");
}

void wholth::remove_food(wholth::entity::food::id_t food_id, sqlw::Connection& con)
{
	using sqlw::status::is_ok;

	sqlw::Statement stmt {&con};

	stmt("SAVEPOINT remove_food_pnt");

	try {
		stmt.prepare("DELETE FROM food WHERE id = :1")
			.bind(1, food_id, sqlw::Type::SQL_INT)
			.exec();
		}
	catch (const std::exception& e) {
		stmt("ROLLBACK TO remove_food_pnt");
		return;
	}

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO remove_food_pnt");
	}

	stmt("RELEASE remove_food_pnt");
}

void wholth::add_nutrients(
	wholth::entity::food::id_t food_id,
	const std::span<wholth::entity::editable::food::Nutrient>& nutrients,
	sqlw::Connection& con
)
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
	const std::span<wholth::entity::editable::food::Nutrient>& nutrients,
	sqlw::Connection& con
)
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
	const std::span<wholth::entity::editable::food::Nutrient>& nutrients,
	sqlw::Connection& con
)
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
	const std::span<wholth::entity::editable::food::RecipeStep>& steps,
	sqlw::Connection& con,
	wholth::entity::locale::id_t locale_id
)
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
	const std::span<wholth::entity::editable::food::RecipeStep>& steps,
	sqlw::Connection& con
)
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
	const std::span<wholth::entity::editable::food::RecipeStep>& steps,
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
	const std::span<wholth::entity::editable::food::Ingredient>& foods,
	sqlw::Connection& con
)
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
	const std::span<wholth::entity::editable::food::Ingredient>& foods,
	sqlw::Connection& con
)
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
	const std::span<wholth::entity::editable::food::Ingredient>& foods,
	sqlw::Connection& con
)
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
