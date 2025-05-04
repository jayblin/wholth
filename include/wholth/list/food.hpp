#ifndef WHOLTH_LIST_FOOD_H_
#define WHOLTH_LIST_FOOD_H_

#include <array>
#include <cstdint>
#include <string>

namespace wholth::list::food
{
	/* namespace nutrient_filter */
	/* { */
	/* 	enum class Operation */
	/* 	{ */
	/* 		EQ, */
	/* 		NEQ, */
	/* 		BETWEEN, */
	/* 	}; */

        /* // todo remove wholth::entity::nutrient::view::id::value_type */
	/* 	typedef TupleElement<wholth::entity::nutrient::view::id::value_type, 1> NutrientId; */
	/* 	typedef TupleElement<std::string_view, 2> Value; */

	/* 	typedef std::tuple< */
	/* 		nutrient_filter::Operation, */
	/* 		NutrientId::value_type, */
	/* 		Value::value_type */
	/* 	> Entry; */
	/* }; */

	/* struct Query */
	/* { */
        /* /1* template <typename T> *1/ */
        /* /1* Query(const T& model); *1/ */

        /* /1* Query() {} *1/ */

	/* 	uint64_t page {0}; */
	/* 	std::string_view locale_id {""}; */
	/* 	/** */
	/* 	 * Food's title. */
	/* 	 *1/ */
	/* 	std::string_view title {""}; */
	/* 	/** */
	/* 	 * Coma separated list of ingredient names. */
	/* 	 *1/ */
	/* 	std::string_view ingredients {""}; */
	/* 	std::span<nutrient_filter::Entry> nutrient_filters {}; */
	/* }; */

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
}


#endif // WHOLTH_LIST_FOOD_H_
