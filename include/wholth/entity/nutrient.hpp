#ifndef WHOLTH_ENTITY_NUTRIENT_H_
#define WHOLTH_ENTITY_NUTRIENT_H_

#include <string_view>
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
/* #include "wholth/pager.hpp" */
#include "wholth/entity/utils.hpp"

namespace wholth::entity::nutrient
{
	namespace view
	{
		typedef TupleElement<std::string_view, 0> id;
		typedef TupleElement<std::string_view, 1> title;
		typedef TupleElement<std::string_view, 2> value;
		typedef TupleElement<std::string_view, 3> unit;
		typedef TupleElement<std::string_view, 4> alias;
		typedef TupleElement<std::string_view, 5> description;
	};

	typedef std::tuple<
		view::id::value_type,
		view::title::value_type,
		view::unit::value_type,
		view::alias::value_type,
		view::description::value_type
	> View;
};

namespace wholth {

	/* struct NutrientsQuery */
	/* { */
	/* 	uint32_t limit {20}; */
	/* 	uint32_t page {0}; */
	/* 	// @todo: programmaticlay resolve default locale_id. */
	/* 	std::string_view locale_id {""}; */
	/* 	std::string_view title; */
	/* }; */

	/* template<> template<> */
	/* PaginationInfo Pager<entity::nutrient::View>::query<NutrientsQuery>( */
	/* 	std::span<entity::nutrient::View>, */
	/* 	sqlw::Connection*, */
	/* 	const NutrientsQuery& */
	/* ); */
}

#endif // WHOLTH_ENTITY_NUTRIENT_H_
