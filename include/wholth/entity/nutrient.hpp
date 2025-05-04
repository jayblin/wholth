#ifndef WHOLTH_ENTITY_NUTRIENT_H_
#define WHOLTH_ENTITY_NUTRIENT_H_

#include <string_view>
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

#endif // WHOLTH_ENTITY_NUTRIENT_H_
