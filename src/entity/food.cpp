#include "wholth/entity/food.hpp"

entity::food::Input entity::food::input::initialize()
{
return {{"##food_title"}, {"##food_calories"}};
}

static std::string create_entity_query_sql(
	const FoodsQuery& q
)
{
	std::stringstream stream;
	std::stringstream where_stream;
	std::string_view join;

	if (q.title.size() > 0)
	{
		where_stream << "WHERE fl.title LIKE ?2 ";
		join = " LEFT JOIN food_localisation fl "
		   "ON fl.food_id = f.id ";
	}
	else
	{
		join = " LEFT JOIN food_localisation fl "
		   "ON fl.food_id = f.id AND fl.locale_id = ?1 ";
	}

	auto where = where_stream.str();

	stream
		<< '%' << q.title << '%'
		<< "SELECT "
		   "f.id, "
		   "CASE WHEN fl.id IS NOT NULL THEN fl.title ELSE '[N/A]' END AS title, "
		   "f.calories "
		<< "FROM food f "
		<< join
		<< where
		<< "ORDER BY f.id, fl.title "
		<< "LIMIT " << q.limit << ' '
		<< "OFFSET " << (q.limit * q.page);

	return stream.str();
}

static std::string create_pagination_query_sql(const FoodsQuery& q)
{
	std::stringstream stream;
	std::stringstream where_stream;
	std::string_view join;

	if (q.title.size() > 0)
	{
		where_stream << "WHERE fl.title LIKE ?2 ";
		join = "LEFT JOIN food_localisation fl "
		   "ON fl.food_id = f.id ";
	}
	else
	{
		join = " LEFT JOIN food_localisation fl "
		   "ON fl.food_id = f.id AND fl.locale_id = ?1 ";
	}

	auto where = where_stream.str();

	stream
		<< "SELECT "
			"r.cnt AS cnt, "
			"MAX(1, CAST(ROUND(CAST(r.cnt AS float) / " << q.limit << " + 0.49) AS int)) AS max_page, "
			<< '\'' << (q.page + 1) << "/' || (MAX(1, CAST(ROUND(CAST(r.cnt AS float) / " << q.limit << " + 0.49) AS int))) AS paginator_str "
			"FROM ( "
				"SELECT "
				"COUNT(f.id) AS cnt "
				"FROM food f "
			<< join
			<< where
			<< ") r "
	;

	return stream.str();
}

template<> template<>
PaginationInfo ViewList<entity::food::View>::query_page<FoodsQuery>(
	sqlw::Connection* con,
	const FoodsQuery& q
)
{
	PaginationInfo p_info {};

	size_t buffer_idx = (m_buffer_idx + 1) % 2;

	if (m_list.size() != q.limit)
	{
		m_list.clear();
		// @todo read about resizing.
		m_list.resize(q.limit);
	}

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

	stmt.bind(1, q.locale_id, sqlw::Type::SQL_INT);

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

	stmt.bind(1, q.locale_id, sqlw::Type::SQL_INT);

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
		m_list.size(),
		i / field_count
	);
	uint32_t offset = 0;

	for (size_t j = 0; j < m_list.size(); j++)
	{
		if (j >= total_fetched)
		{
			m_list[j] = {};
			continue;
		}

		entity::food::View entry;

		std::apply(
			[&offset, &lengths, &buffer_ref, j, &entry](auto&&... args)
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

		m_list[j] = entry;
	}

	for (size_t o = 0; o < this->size(); o++)
	{
		const auto& l = m_list[o];
		if (entity::get<entity::food::view::id>(l).size() == 0)
		{
			break;
		}
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
