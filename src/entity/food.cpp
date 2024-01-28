#include "wholth/entity/food.hpp"
#include "fmt/color.h"
#include "fmt/core.h"
#include "sqlw/forward.hpp"
#include "sqlw/status.hpp"
#include "wholth/entity/nutrient.hpp"
#include <charconv>
#include <sstream>
#include <string>

wholth::entity::food::Input wholth::entity::food::input::initialize()
{
	return {{"##food_title"}, {"##food_calories"}};
}

static std::string_view create_join(const wholth::FoodsQuery& q)
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
		"ON fn.food_id = f.id ";
	}

	return "LEFT JOIN food_localisation AS fl "
	   "ON fl.food_id = f.id AND fl.locale_id = :1 "
		"LEFT JOIN food_nutrient AS fn "
		"ON fn.food_id = f.id ";
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
			"CASE WHEN fl.title IS NOT NULL THEN fl.title ELSE '[N/A]' END AS title, "
			"CASE WHEN fl.description IS NOT NULL THEN fl.description ELSE '[N/A]' END AS description "
		"FROM food f "
		" {1} "
		" {2} "
		"GROUP BY f.id "
		"ORDER BY f.id ASC, fl.locale_id ASC, fl.title ASC "
		"LIMIT {3} "
		"OFFSET {4}",
		q.title,
		create_join(q),
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
		create_join(q),
		create_where(q)
	);
}

template<> template<>
PaginationInfo wholth::Pager<wholth::entity::food::View>::query_page<wholth::FoodsQuery>(
	std::span<entity::food::View> span,
	sqlw::Connection* con,
	const FoodsQuery& q
)
{
	PaginationInfo p_info {};

	size_t buffer_idx = (m_buffer_idx + 1) % 2;

	sqlw::Statement entity_stmt {con};
	sqlw::Statement paginator_stmt {con};
	constexpr auto field_count = std::tuple_size_v<entity::food::View>;

	auto entity_sql = create_entity_query_sql(q);
	auto paginator_sql = create_pagination_query_sql(q);
	
	std::stringstream buffer_stream;
	// To store the start and end of values.
	std::vector<uint32_t> lengths (
		q.limit * field_count // number of fields in SELECT
	);

	uint32_t i = 0;

	entity_stmt.prepare(entity_sql);

	bind_params(entity_stmt, q);

	entity_stmt([&buffer_stream, &lengths, &i](sqlw::Statement::ExecArgs e)
		{
			buffer_stream << e.column_value;

			lengths[i] = e.column_value.size();
			i++;
		}
	);

	paginator_stmt.prepare(paginator_sql);

	bind_params(paginator_stmt, q);

	size_t count_len = 0;
	size_t max_page_len = 0;
	size_t paginator_str_len = 0;

	paginator_stmt([&buffer_stream, &count_len, &max_page_len, &paginator_str_len](sqlw::Statement::ExecArgs e)
		{
			buffer_stream << e.column_value;

			if ("cnt" == e.column_name)
			{
				count_len = e.column_value.size();
			}
			else if ("max_page" == e.column_name)
			{
				max_page_len = e.column_value.size();
			}
			else if ("paginator_str" == e.column_name)
			{
				paginator_str_len = e.column_value.size();
			}
		}
	);

	auto s = buffer_stream.str();

	m_buffers[buffer_idx] = std::move(s);

	auto& buffer_ref = m_buffers[buffer_idx];

	if (!(buffer_ref.size() > 0) || !(count_len > 0))
	{
		return p_info;
	}

	size_t total_fetched = std::min(
		span.size(),
		i / field_count
	);
	uint32_t offset = 0;

	for (size_t j = 0; j < span.size(); j++)
	{
		if (j >= total_fetched)
		{
			span[j] = {};
			continue;
		}

		entity::food::View entry;

		std::apply(
			[&offset, &lengths, &buffer_ref, j](auto&&... args)
			{
				size_t k = 0;
				(
					[&k, &args, &offset, &lengths, &buffer_ref, j]{
						args = {
							buffer_ref.data() + offset,
							lengths[j * field_count + k]
						};
						offset += lengths[j * field_count + k];
						k++;
					} (),
					...
				);
			},
			entry
		);

		span[j] = entry;
	}

	p_info.element_count = {
		buffer_ref.data() + offset,
		count_len
	};

	offset += count_len;

	p_info.max_page = {
		buffer_ref.data() + offset,
		max_page_len
	};

	offset += max_page_len;

	p_info.progress_string = {
		buffer_ref.data() + offset,
		paginator_str_len
	};

	offset += paginator_str_len;

	m_buffer_idx = buffer_idx;

	return p_info;
}
