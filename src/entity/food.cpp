#include "wholth/entity/food.hpp"
#include "fmt/core.h"
#include <sstream>

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
		return "LEFT JOIN food_localisation fl "
		   "ON fl.food_id = f.id ";
	}

	return "LEFT JOIN food_localisation fl "
	   "ON fl.food_id = f.id AND fl.locale_id = ?1 ";
}

static std::string create_entity_query_sql(
	const wholth::FoodsQuery& q
)
{
	return fmt::format(
		"%{0}%"
		"SELECT "
			"f.id, "
			"CASE WHEN fl.title IS NOT NULL THEN fl.title ELSE '[N/A]' END AS title, "
			"CASE WHEN fl.description IS NOT NULL THEN fl.description ELSE '[N/A]' END AS title "
		"FROM food f "
		" {1} "
		" {2} "
		"ORDER BY f.id, fl.title "
		"LIMIT {3} "
		"OFFSET {4}",
		q.title,
		create_join(q),
		q.title.size() > 0 ? " WHERE fl.title LIKE ?2  " : "",
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
			"COUNT(f.id) AS cnt "
			"FROM food f "
			" {3} "
			" {4} "
		") AS r",
		q.limit,
		q.page + 1,
		q.limit,
		create_join(q),
		q.title.size() > 0 ? " WHERE fl.title LIKE ?2 " : ""
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

	sqlw::Statement stmt {con};
	constexpr auto field_count = std::tuple_size_v<entity::food::View>;

	auto entity_sql = create_entity_query_sql(q);
	auto paginator_sql = create_pagination_query_sql(q);
	
	std::stringstream buffer_stream;
	// To store the start and end of values.
	std::vector<uint32_t> lengths (
		q.limit * field_count // number of fields in SELECT
	);

	uint32_t i = 0;

	stmt.prepare({entity_sql.data() + q.title.size() + 2});

	if (q.locale_id.size() > 0)
	{
		stmt.bind(1, q.locale_id, sqlw::Type::SQL_INT);
	}

	if (q.title.size() > 0)
	{
		stmt.bind(
			2,
			{entity_sql.data(), q.title.size() + 2},
			sqlw::Type::SQL_TEXT
		);
	}

	stmt([&buffer_stream, &lengths, &i](sqlw::Statement::ExecArgs e)
		{
			buffer_stream << e.column_value;

			lengths[i] = e.column_value.size();
			i++;
		}
	);

	size_t count_len = 0;
	size_t max_page_len = 0;
	size_t paginator_str_len = 0;

	stmt.prepare(paginator_sql);

	if (q.locale_id.size() > 0)
	{
		stmt.bind(1, q.locale_id, sqlw::Type::SQL_INT);
	}

	if (q.title.size() > 0)
	{
		stmt.bind(
			2,
			{entity_sql.data(), q.title.size() + 2},
			sqlw::Type::SQL_TEXT
		);
	}

	stmt([&buffer_stream, &count_len, &max_page_len, &paginator_str_len](sqlw::Statement::ExecArgs e)
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
