#ifndef WHOLTH_LIST_FOOD_H_
#define WHOLTH_LIST_FOOD_H_

#include "sqlw/connection.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/status.hpp"
#include <cstdint>
#include <span>
#include <string_view>
#include <system_error>

namespace wholth::list::food
{
    typedef std::string buffer_t;

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
	};

	struct Query
	{
		uint64_t page {0};
		std::string_view locale_id {""};
		/**
		 * Food's title.
		 */
		std::string_view title {""};
		/**
		 * Coma separated list of ingredient names.
		 */
		std::string_view ingredients {""};
		std::span<nutrient_filter::Entry> nutrient_filters {};
	};

    struct IngredientData
    {
        uint64_t parameter_count;
        /**
         * A string like "%protein%%calori%"
         */
		std::string parameter_buffer;
        /**
         * Number of chars for each parameter om...
         */
		std::array<uint64_t, 32> parameter_lengths;
        /**
         * Clauses for WHERE statement to filter ingredients.
         */
		std::string where;
    };

    auto list(
        std::span<wholth::entity::shortened::Food>,
        uint64_t& count,
        buffer_t&,
        const Query&,
        sqlw::Connection&
    ) -> std::error_code;
}

#endif // WHOLTH_LIST_FOOD_H_
