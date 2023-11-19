#ifndef WHOLTH_ENTITY_FOOD_H_
#define WHOLTH_ENTITY_FOOD_H_

#include <string_view>
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "wholth/view_list.hpp"
#include "wholth/entity/utils.hpp"

namespace entity::food
{
	namespace view
	{
		typedef TupleElement<std::string_view, 0> id;
		typedef TupleElement<std::string_view, 1> title;
		typedef TupleElement<std::string_view, 2> calories;
	};

	typedef std::tuple<
		view::id::value_type,
		view::title::value_type,
		view::calories::value_type
	> View;

	namespace input
	{
		typedef TupleElement<TextInput<255>, 0> title;
		typedef TupleElement<FloatInput, 1> calories;
	};

	typedef std::tuple<
		input::title::value_type,
		input::calories::value_type
	> Input;

	namespace input
	{
		Input initialize();
	};
};

struct FoodsQuery
{
	uint32_t limit {20};
	uint32_t page {0};
	std::string_view locale_id {"0"};
	std::string_view title;
};

template<> template<>
PaginationInfo ViewList<entity::food::View>::query_page<FoodsQuery>(
	sqlw::Connection*,
	const FoodsQuery&
);

#endif // WHOLTH_ENTITY_FOOD_H_
