#ifndef WHOLTH_LIST_FOOD_H_
#define WHOLTH_LIST_FOOD_H_

#include "wholth/entity/food.hpp"
#include <span>
#include <string_view>

namespace wholth::list::food
{
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
		u_int32_t page {0};
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

	typedef std::tuple<
		size_t,
		std::string,
		std::string,
		std::array<size_t, 32>
	> IngredientData;

	/**
	 * @stateful
	 */
	class Lister
	{
		public:
			typedef std::string buffer_t;

			Lister(const Query& q, sqlw::Connection* db_con):
				m_q(q),
				m_ingredient_data(prepare_ingredient_data(q)), m_db_con(db_con)
			{
			}

			auto list(
				std::span<wholth::entity::shortened::Food>,
				buffer_t&
			) -> wholth::StatusCode;

			auto count(buffer_t&) -> wholth::StatusCode;

			auto pagination(
				PaginationInfo&,
				buffer_t&,
				size_t list_size
			) -> wholth::StatusCode;
		private:
			const Query& m_q;
			IngredientData m_ingredient_data;
			sqlw::Connection* m_db_con;

			auto prepare_ingredient_data(
				const Query& q
			) -> IngredientData;
	};
}


#endif // WHOLTH_LIST_FOOD_H_
