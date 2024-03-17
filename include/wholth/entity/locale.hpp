#ifndef WHOLTH_ENTITY_LOCALE_H_
#define WHOLTH_ENTITY_LOCALE_H_

#include "wholth/entity/utils.hpp"
#include <string_view>
namespace wholth::entity::locale
{
	typedef std::string_view id_t;
	typedef std::string_view alias;

	namespace view
	{
		typedef TupleElement<std::string_view, 0> id;
		typedef TupleElement<std::string_view, 1> alias;
	};
};

#endif // WHOLTH_ENTITY_LOCALE_H_
