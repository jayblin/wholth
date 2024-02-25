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
#include <sstream>
#include <stdexcept>
#include <string>

/* wholth::entity::food::Input wholth::entity::food::input::initialize() */
/* { */
/* 	return {{"##food_title"}, {"##food_calories"}}; */
/* } */

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
	const wholth::FoodsQuery& q
)
{
	return fmt::format(
		"SELECT "
			"f.id, "
			/* "CASE WHEN fl.title IS NOT NULL THEN fl.title ELSE '[N/A]' END AS title, " */
			"COALESCE(fl.title, '[N/A]') AS title, "
			/* "CASE WHEN fl.description IS NOT NULL THEN fl.description ELSE '[N/A]' END AS description " */
			"COALESCE("
				"CASE WHEN rs.time IS NULL THEN rs.time "
				"WHEN rs.time < 60 THEN ((rs.time) || 's') "
				"WHEN rs.time < 3600 THEN ((rs.time / 60) || 'm') "
				"ELSE ((rs.time / 3600) || 'h') "
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
		q.limit,
		q.limit * q.page
	);
}

static std::string create_pagination_query_sql(const wholth::FoodsQuery& q)
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
		q.limit,
		q.page + 1,
		q.limit,
		create_joins(q),
		create_where(q)
	);
}

/* template<typename T> template<typename Q> */
/* PaginationInfo wholth::Pager<T>::query_page( */
/* 	std::span<T> span, */
/* 	sqlw::Connection* con, */
/* 	const Q& q */
/* ) */
/* { */
/* 	PaginationInfo p_info {}; */

/* 	size_t buffer_idx = (m_buffer_idx + 1) % 2; */

/* 	sqlw::Statement entity_stmt {con}; */
/* 	sqlw::Statement paginator_stmt {con}; */

/* 	auto entity_sql = this->create_entity_query_sql(q); */
/* 	auto paginator_sql = this->create_pagination_query_sql(q); */
	
/* 	std::stringstream buffer_stream; */

/* 	wholth::utils::LengthContainer lengths { */
/* 		3 // same as count of fields in PaginationInfo struct */
/* 		+ (q.limit * this->field_count()) */
/* 	}; */

/* 	paginator_stmt.prepare(paginator_sql); */

/* 	bind_params(paginator_stmt, q); */

/* 	paginator_stmt([&buffer_stream, &lengths](sqlw::Statement::ExecArgs e) */
/* 		{ */
/* 			buffer_stream << e.column_value; */

/* 			lengths.add(e.column_value.size()); */
/* 		} */
/* 	); */

/* 	entity_stmt.prepare(entity_sql); */

/* 	bind_params(entity_stmt, q); */

/* 	entity_stmt([&buffer_stream, &lengths](sqlw::Statement::ExecArgs e) */
/* 		{ */
/* 			buffer_stream << e.column_value; */

/* 			lengths.add(e.column_value.size()); */
/* 		} */
/* 	); */

/* 	auto s = buffer_stream.str(); */

/* 	m_buffers[buffer_idx] = std::move(s); */

/* 	auto& buffer_ref = m_buffers[buffer_idx]; */

/* 	if (!(buffer_ref.size() > 0)) */
/* 	{ */
/* 		return p_info; */
/* 	} */

/* 	p_info.element_count = lengths.next(buffer_ref); */
/* 	p_info.max_page = lengths.next(buffer_ref); */
/* 	p_info.progress_string = lengths.next(buffer_ref); */

/* 	this->populate_span(span, lengths); */

/* 	m_buffer_idx = buffer_idx; */

/* 	return p_info; */
/* } */

/* template<> */
/* constexpr size_t wholth::Pager<wholth::entity::viewable::Food>::field_count() */
/* { */
/* 	return 3; */
/* } */

/* template<> */
/* void wholth::Pager<wholth::entity::viewable::Food>::populate_span( */
/* 	std::span<wholth::entity::viewable::Food> span, */
/* 	const std::string& buffer_ref, */
/* 	wholth::utils::LengthContainer& lengths */
/* ) */
/* { */
/* 	size_t j = 0; */
/* 	for (std::string_view id = lengths.next(buffer_ref); id != wholth::utils::NIL; id = lengths.next(buffer_ref)) */
/* 	{ */
/* 		wholth::entity::viewable::Food entry; */

/* 		entry.id = id; */
/* 		entry.title = lengths.next(buffer_ref); */
/* 		entry.preparation_time = lengths.next(buffer_ref); */

/* 		span[j] = entry; */
/* 		j++; */
/* 	} */
/* } */


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

	auto entity_sql = create_entity_query_sql(q);
	auto paginator_sql = create_pagination_query_sql(q);
	
	std::stringstream buffer_stream;

	wholth::utils::LengthContainer itr {
		(q.limit * field_count) + 3
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

	/* if (!(buffer_ref.size() > 0) || !(count_len > 0)) */
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

static std::string insert_food(
	sqlw::Statement& stmt,
	const std::string_view now
)
{
	std::string food_id;

	stmt.prepare(
			"INSERT INTO food (created_at) VALUES (:1); "
		)
		.bind(1, now)
		.exec();

	if (!sqlw::status::is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return food_id;
	}

	stmt(
		"SELECT last_insert_rowid()",
		[&food_id](sqlw::Statement::ExecArgs e) {
			food_id = e.column_value;
		}
	);

	return food_id;
}

static void insert_food_localisation(
	sqlw::Statement& stmt,
	const std::string_view food_id,
	const std::string_view locale_id,
	const wholth::entity::editable::Food& food
)
{
	stmt.prepare(
			"INSERT INTO food_localisation (food_id,locale_id,title,description) "
			"VALUES (:1,:2,:3,:4)"
		)
		.bind(1, food_id)
		.bind(2, locale_id)
		.bind(3, food.title)
		.bind(4, food.description)
		.exec();
}

static void insert_food_nutrients(
	sqlw::Statement& stmt,
	const std::string_view food_id,
	const std::string_view now,
	const wholth::entity::editable::Food& food
)
{
	if (!(
		food.nutrients.size() > 0
		&& food.nutrients[0].value.length() > 0
	))
	{
		return;
	}

	wholth::utils::IndexSequence<4, 1> idxs{};
	std::stringstream ss;

	ss <<
		" INSERT INTO food_nutrient (food_id,nutrient_id,value,created_at) "
		"VALUES ";

	/* for (const wholth::entity::food::food_specific::Nutrient& nutrient : food.nutrients) { */
	for (const auto& nutrient : food.nutrients) {
		if (nutrient.value.length() == 0) {
			break;
		}

		ss << fmt::format(
			"(:{},:{},:{},:{}),",
			idxs[0],
			idxs[1],
			idxs[2],
			idxs[3]
		);

		idxs.advance();
	}

	std::string sql = ss.str();
	/* fmt::print(fmt::fg(fmt::color::green), "{}\n", sql); */
	
	stmt.prepare(sql.substr(0, sql.size() - 1));

	idxs.reset();

	/* for (const wholth::entity::food::food_specific::Nutrient& nutrient : food.nutrients) { */
	for (const auto& nutrient : food.nutrients) {

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

		stmt.bind(idxs[3], now, sqlw::Type::SQL_TEXT);

		idxs.advance();
	}

	stmt.exec();
}

static void insert_food_steps(
	sqlw::Statement& stmt,
	const std::string_view food_id,
	const std::string_view locale_id,
	const wholth::entity::editable::Food& food
)
{
	if (!(food.steps.size() > 0))
	{
		return;
	}

	stmt
		.prepare("INSERT INTO recipe_step (recipe_id,time) VALUES (:1,:2)")
		.bind(1, food_id)
		.bind(2, food.steps[0].time)
		.exec();

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
		.bind(3, food.steps[0].description)
		.exec();

	if (!(food.steps[0].foods.size() > 0))
	{
		return;
	}

	std::stringstream ss;
	wholth::utils::IndexSequence<3, 1> idxs;

	ss << "INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) "
		"VALUES ";

	for (const auto& step_food : food.steps[0].foods)
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
	/* fmt::print(fmt::fg(fmt::color::green), "{}\n", sql); */
	
	stmt.prepare(sql.substr(0, sql.size() - 1));

	idxs.reset();

	for (const auto& step_food : food.steps[0].foods)
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

void wholth::insert(
	sqlw::Connection& con,
	/* const wholth::entity::food::Input& food, */
	const wholth::entity::editable::Food& food,
	const wholth::entity::locale::view::id::value_type& locale_id
)
{
	using sqlw::status::is_ok;

	sqlw::Statement stmt {&con};
	stmt("SAVEPOINT insert_food_pnt");

	const std::string now = wholth::utils::current_time_and_date();
	std::string food_id = insert_food(stmt, now);

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return;
	}

	insert_food_localisation(stmt, food_id, locale_id, food);

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return;
	}

	insert_food_nutrients(stmt, food_id, now, food);

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return;
	}

	insert_food_steps(stmt, food_id, locale_id, food);

	if (!is_ok(stmt.status()))
	{
		stmt("ROLLBACK TO insert_food_pnt");
		return;
	}

	stmt("RELEASE insert_food_pnt");
}
