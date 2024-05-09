#include "wholth/list/food.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/utils.hpp"

using SC = wholth::StatusCode;

namespace whf = wholth::list::food;

static bool is_empty(const whf::nutrient_filter::Entry& entry)
{
	return !(std::get<whf::nutrient_filter::Value::index>(entry).length() > 0);
}

static std::string create_where(
	const whf::Query& q,
	size_t sql_param_idx
)
{
	using OP = whf::nutrient_filter::Operation;

	std::stringstream tpl_stream;
	/* size_t sql_param_idx = 2; */

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
					" ON fn{}.food_id = rt.recipe_id "
					" AND fn{}.nutrient_id = ?{} "
					" AND (fn{}.value <= (?{} + 0.001) AND fn{}.value >= (?{} - 0.001)) ",
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
					" ON fn{}.food_id = rt.recipe_id "
					" AND fn{}.nutrient_id = ?{} "
					" AND (fn{}.value < (?{} - 0.001) or fn{}.value > (?{} + 0.001)) ",
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
					" ON fn{}.food_id = rt.recipe_id "
					" AND fn{}.nutrient_id = ?{} "
					" AND (fn{}.value >= ?{} AND fn{}.value <= ?{}) ",
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
			" rt.recipe_title LIKE ?{} ",
			sql_param_idx
		);
		sql_param_idx++;
	}

	return tpl_stream.str();
}

auto whf::Lister::prepare_ingredient_data(
	const Query& q
) -> IngredientData {
	std::stringstream where_stream;
	std::stringstream tokens_stream;
	size_t start = 0;
	std::array<size_t, 32> lengths;
	std::fill(lengths.begin(), lengths.end(), 0);

	// Starting from 1, because first one is always locale_id.
	size_t param_idx = 1;
	for (size_t i = 0; i < q.ingredients.size() && i < lengths.size(); i++) {
		const char ch = q.ingredients[i];
		size_t j = i + 1;
		std::string_view sub;

		// Extract the word excluding:
		// - ',';
		// - all spaces before the word.
		if (ch == ',') {
			if (start == 0) {
				sub = q.ingredients.substr(start, i - start);
			}
			else {
				auto first_letter_idx = q.ingredients.find_first_not_of(' ', start);
				sub = q.ingredients.substr(first_letter_idx, i - first_letter_idx);
			}
			start = i + 1;
		} else if (q.ingredients.size() == j) {
			auto first_letter_idx = q.ingredients.find_first_not_of(' ', start);
			sub = q.ingredients.substr(first_letter_idx, j - first_letter_idx);
		}

		if (sub.size() > 2) {
			lengths[param_idx - 1] = sub.size() + 2;
			param_idx++;
			where_stream << "fl.title LIKE ?" << param_idx << " OR ";
			tokens_stream << '%' << sub << '%';
		}
	}

	std::string where = "1";

	if (where_stream.rdbuf()->in_avail() > 4) {
		where = where_stream.str();
		where = where.substr(0, where.size() - 4);
	}

	return {
		param_idx,
		where,
		tokens_stream.str(),
		lengths
	};
}

// @todo rename `idx`.
static void bind_params(
	sqlw::Statement& stmt,
	const whf::Query& q,
	size_t idx
)
{
	using OP = whf::nutrient_filter::Operation;

	for (const whf::nutrient_filter::Entry& entry : q.nutrient_filters)
	{
		if (is_empty(entry))
		{
			break;
		}

		const auto& value = std::get<whf::nutrient_filter::Value::index>(entry);

		switch (std::get<OP>(entry))
		{
			case OP::EQ: {
				stmt.bind(
					idx,
					std::get<whf::nutrient_filter::NutrientId::index>(entry),
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
					std::get<whf::nutrient_filter::NutrientId::index>(entry),
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
					std::get<whf::nutrient_filter::NutrientId::index>(entry),
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

auto list_foods_prepare_stmt(
	std::string_view sql,
	const whf::Query& q,
	const whf::IngredientData& ingredient_data,
	sqlw::Connection* con
) -> sqlw::Statement
{
	const auto& [ingredient_param_count, ingredient_where, ingredient_tokens, ingredient_tokens_lengths] = ingredient_data;
	size_t param_idx = ingredient_param_count;
	/* fmt::print("{}\n", sql); */
	
	sqlw::Statement stmt {con};
	stmt.prepare(sql);

	size_t ing_start = 0;

	param_idx = 1;

	stmt.bind(param_idx, q.locale_id, sqlw::Type::SQL_INT);
	param_idx++;

	for (size_t i = 0; i < ingredient_tokens_lengths.size(); i++) {

		if (0 == ingredient_tokens_lengths[i]) {
			break;
		}

		stmt.bind(
			param_idx,
			std::string_view{ingredient_tokens.data() + ing_start, ingredient_tokens_lengths[i]},
			sqlw::Type::SQL_TEXT
		);
		ing_start += ingredient_tokens_lengths[i];
		param_idx++;
	}

	bind_params(stmt, q, param_idx);

	return stmt;
}

auto whf::Lister::list(
	std::span<wholth::entity::shortened::Food> span,
	whf::Lister::buffer_t& buffer
) -> wholth::StatusCode
{
	if (m_q.locale_id.size() == 0 || !sqlw::utils::is_numeric(m_q.locale_id)) {
		return SC::INVALID_LOCALE_ID;
	}

	std::stringstream buffer_stream;

	// @todo: rethink this jank!
	constexpr size_t field_count = 4;
	wholth::utils::LengthContainer itr {
		(span.size() * field_count)
	};
	const auto& [ingredient_param_count, ingredient_where, ingredient_tokens, ingredient_tokens_lengths] = m_ingredient_data;
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
				 ON fl.food_id = f.id AND fl.locale_id = ?1
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
				ON fl.food_id = node.recipe_id AND fl.locale_id = ?1
			ORDER BY 1 DESC
		)
		SELECT
			rt.recipe_id AS id,
			COALESCE(rt.recipe_title, '[N/A]') AS title,
			COALESCE(
				seconds_to_readable_time(rt.step_seconds),
				'[N/A]'
			) AS time,
			COALESCE(mvfn.value, '[N/A]') || ' ' || COALESCE(mvn.unit, '') AS top_nutrient
		FROM recipe_tree rt
		LEFT JOIN food_nutrient mvfn
			ON mvfn.food_id = rt.recipe_id
		LEFT JOIN nutrient mvn
			ON mvn.position = 0 AND mvn.id = mvfn.nutrient_id
		{1}
		GROUP BY rt.recipe_id
		ORDER BY rt.lvl DESC, rt.recipe_id ASC, rt.recipe_title ASC
		LIMIT {2}
		OFFSET {3}
	)sql";

	auto stmt = list_foods_prepare_stmt(
		fmt::format(
			sql,
			ingredient_where,
			create_where(m_q, ingredient_param_count + 1),
			span.size(),
			span.size() * m_q.page
		),
		m_q,
		m_ingredient_data,
		m_db_con
	);

	stmt([&buffer_stream, &itr](sqlw::Statement::ExecArgs e)
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

	for (size_t j = 0; j < span.size(); j ++)
	{
		wholth::entity::shortened::Food entry;

		entry.id = itr.next(buffer);
		entry.title = itr.next(buffer);
		entry.preparation_time = itr.next(buffer);
		entry.top_nutrient = itr.next(buffer);

		span[j] = entry;
	}

	return SC::NO_ERROR;
}

auto whf::Lister::count(
	whf::Lister::buffer_t& buffer
) -> wholth::StatusCode
{
	if (m_q.locale_id.size() == 0 || !sqlw::utils::is_numeric(m_q.locale_id)) {
		return SC::INVALID_LOCALE_ID;
	}

	std::stringstream buffer_stream;

	const auto& [ingredient_param_count, ingredient_where, ingredient_tokens, ingredient_tokens_lengths] = m_ingredient_data;
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
				ON fl.food_id = f.id AND fl.locale_id = ?1
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
				ON fl.food_id = node.recipe_id AND fl.locale_id = ?1
			ORDER BY 1 DESC
		)
		SELECT
			COUNT(g.id) AS cnt
		FROM (
			SELECT
				rt.recipe_id AS id
			FROM recipe_tree rt
			LEFT JOIN food_nutrient mvfn
				ON mvfn.food_id = rt.recipe_id
			LEFT JOIN nutrient mvn
				ON mvn.position = 0 AND mvn.id = mvfn.nutrient_id
			{1}
			GROUP BY rt.recipe_id
			ORDER BY rt.lvl DESC, rt.recipe_id ASC, rt.recipe_title ASC
		) AS g
	)sql";

	auto stmt = list_foods_prepare_stmt(
		fmt::format(
			sql,
			ingredient_where,
			create_where(m_q, ingredient_param_count + 1)
		),
		m_q,
		m_ingredient_data,
		m_db_con
	);

	stmt([&buffer_stream](sqlw::Statement::ExecArgs e)
		{
			buffer_stream << e.column_value;
		}
	);

	if (!(buffer_stream.rdbuf()->in_avail() > 0))
	{
		// todo test
		return SC::ENTITY_NOT_FOUND;
	}

	buffer = buffer_stream.str();

	return SC::NO_ERROR;
}

auto whf::Lister::pagination(
	PaginationInfo& p_info,
	whf::Lister::buffer_t& buffer,
	size_t list_size
) -> wholth::StatusCode
{
	if (m_q.locale_id.size() == 0 || !sqlw::utils::is_numeric(m_q.locale_id)) {
		return SC::INVALID_LOCALE_ID;
	}

	std::stringstream buffer_stream;

	wholth::utils::LengthContainer itr {3};

	const auto& [ingredient_param_count, ingredient_where, ingredient_tokens, ingredient_tokens_lengths] = m_ingredient_data;
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
				ON fl.food_id = f.id AND fl.locale_id = ?1
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
				ON fl.food_id = node.recipe_id AND fl.locale_id = ?1
			ORDER BY 1 DESC
		)
		SELECT
			r.cnt AS cnt,
			MAX(1, CAST(ROUND(CAST(r.cnt AS float) / {1} + 0.49) as int)) AS max_page,
			'{2}' || '/' || (MAX(1, CAST(ROUND(CAST(r.cnt AS float) / {3} + 0.49) AS int))) AS paginator_str
		FROM (
			SELECT
				COUNT(g.id) AS cnt
			FROM (
				SELECT
					rt.recipe_id AS id
				FROM recipe_tree rt
				LEFT JOIN food_nutrient mvfn
					ON mvfn.food_id = rt.recipe_id
				LEFT JOIN nutrient mvn
					ON mvn.position = 0 AND mvn.id = mvfn.nutrient_id
				{4}
				GROUP BY rt.recipe_id
				ORDER BY rt.lvl DESC, rt.recipe_id ASC, rt.recipe_title ASC
			) AS g
		) AS r
	)sql";

	auto stmt = list_foods_prepare_stmt(
		fmt::format(
			sql,
			ingredient_where,
			list_size,
			m_q.page + 1,
			list_size,
			create_where(m_q, ingredient_param_count + 1)
		),
		m_q,
		m_ingredient_data,
		m_db_con
	);

	stmt([&buffer_stream, &itr](sqlw::Statement::ExecArgs e)
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

	return SC::NO_ERROR;
}
