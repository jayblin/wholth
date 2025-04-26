#ifndef WHOLTH_LIST_NUTRIENT_H_
#define WHOLTH_LIST_NUTRIENT_H_

#include "sqlw/connection.hpp"
#include "sqlw/utils.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/list.hpp"
#include "wholth/status.hpp"
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

namespace wholth::list::nutrient
{
	struct FoodDependentQuery
	{
		uint64_t page {0};
        wholth::entity::locale::id_t locale_id {""};
        wholth::entity::food::id_t food_id {""};
	};
}

#endif // WHOLTH_LIST_NUTRIENT_H_
