#ifndef WHOLTH_ENTITY_FOOD_H_
#define WHOLTH_ENTITY_FOOD_H_

#include <string_view>
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "wholth/entity/nutrient.hpp"
#include "wholth/pager.hpp"
#include "wholth/entity/utils.hpp"

namespace wholth::entity::food
{
	namespace view
	{
		typedef TupleElement<std::string_view, 0> id;
		typedef TupleElement<std::string_view, 1> title;
		typedef TupleElement<std::string_view, 2> description;
	};

	typedef std::tuple<
		view::id::value_type,
		view::title::value_type,
		view::description::value_type
	> View;

	namespace input
	{
		typedef TupleElement<TextInput<255>, 0> title;
		typedef TupleElement<FloatInput, 1> description;
	};

	typedef std::tuple<
		input::title::value_type,
		input::description::value_type
	> Input;

	namespace input
	{
		Input initialize();
	};
};

namespace wholth {

	namespace nutrient_filter
	{
		enum class Operation
		{
			EQ,
			NEQ,
			BETWEEN,
		};

		typedef TupleElement<wholth::entity::nutrient::view::id::value_type, 1> NutrientId;
		typedef TupleElement<std::string_view, 2> Value;

		typedef std::tuple<
			nutrient_filter::Operation,
			NutrientId::value_type,
			Value::value_type
		> Entry;

		/* typedef TupleElement<std::string_view, 2> Value; */

		/* typedef TupleElement<wholth::entity::nutrient::view::id::value_type, 0> NutrientId_; */
		/* typedef TupleElement<std::string_view, 1> Min; */
		/* typedef TupleElement<std::string_view, 2> Max; */
		/* typedef std::tuple< */
		/* 	NutrientId::value_type, */
		/* 	Min::value_type, */
		/* 	Max::value_type */
		/* > Entry_; */
	};

struct FoodsQuery
{
	uint32_t limit {20};
	uint32_t page {0};
	// @todo: programmaticlay resolve default locale_id.
	std::string_view locale_id {""};
	std::string_view title {""};
	std::span<nutrient_filter::Entry> nutrient_filters {};
};

template<> template<>
PaginationInfo Pager<entity::food::View>::query_page<FoodsQuery>(
	std::span<entity::food::View>,
	sqlw::Connection*,
	const FoodsQuery&
);

}

#endif // WHOLTH_ENTITY_FOOD_H_
