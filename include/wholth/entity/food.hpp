#ifndef WHOLTH_ENTITY_FOOD_H_
#define WHOLTH_ENTITY_FOOD_H_

#include <optional>
#include <string>
#include <utility>
#include <initializer_list>
#include <span>
#include <sstream>
#include <string_view>
#include "fmt/core.h"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "wholth/entity/locale.hpp"
#include "wholth/entity/nutrient.hpp"
/* #include "wholth/pager.hpp" */
#include "wholth/entity/utils.hpp"
#include "wholth/pager.hpp"
#include "wholth/utils.hpp"

namespace wholth::entity::food
{
	typedef std::string_view id_t;
	typedef std::string_view title_t;
	typedef std::string_view description_t;
};

namespace wholth::entity::nutrient
{
	typedef std::string_view id_t;
	typedef std::string_view title_t;
	typedef std::string_view value_t;
	typedef std::string_view unit_t;
	typedef std::string_view alias_t;
	typedef std::string_view description_t;
}

namespace wholth::entity::ingredient
{
	typedef std::string_view canonical_mass_t;
};

namespace wholth::entity::recipe_step
{
	typedef std::string_view id_t;
	typedef std::string_view time_t;
	typedef std::string_view note_t;
	typedef std::string_view description_t;
};

namespace wholth::entity::viewable
{
	struct Food
	{
		wholth::entity::food::id_t id;
		wholth::entity::food::title_t title;
		std::string_view preparation_time;
		/* std::string_view health_index; */
	};

	namespace food
	{
		struct Nutrient
		{
			wholth::entity::nutrient::title_t title;
			wholth::entity::nutrient::value_t value;
		};

		struct Ingredient
		{
			wholth::entity::food::title_t title;
			wholth::entity::ingredient::canonical_mass_t canonical_mass;
			size_t ingredient_count {0};
		};

		struct RecipeStep
		{
			wholth::entity::recipe_step::description_t description;
		};

		struct Info
		{
			std::span<RecipeStep> steps;
			std::span<Ingredient> ingredients;
			std::span<Nutrient> nutrients;
		};
	}
}

namespace wholth::entity::editable
{
	namespace food
	{
		struct Nutrient
		{
			wholth::entity::nutrient::id_t id {wholth::utils::NIL};
			wholth::entity::nutrient::value_t value {wholth::utils::NIL};
		};

		struct Ingredient
		{
			wholth::entity::food::id_t food_id {wholth::utils::NIL};
			wholth::entity::ingredient::canonical_mass_t canonical_mass {wholth::utils::NIL};
		};

		struct RecipeStep
		{
			wholth::entity::recipe_step::id_t id {wholth::utils::NIL};
			wholth::entity::recipe_step::time_t seconds {wholth::utils::NIL};
			wholth::entity::recipe_step::description_t description {wholth::utils::NIL};
		};
	};

	struct Food
	{
		wholth::entity::food::id_t id {wholth::utils::NIL};
		wholth::entity::food::title_t title {wholth::utils::NIL};
		wholth::entity::food::description_t description {wholth::utils::NIL};
	};
}

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
	};

	std::optional<wholth::entity::food::id_t> insert_food(
		const wholth::entity::editable::Food&,
		sqlw::Connection&,
		wholth::entity::locale::id_t
	);

	void update_food(
		const wholth::entity::editable::Food&,
		sqlw::Connection&,
		wholth::entity::locale::id_t
	);

	void remove_food(
		wholth::entity::food::id_t,
		sqlw::Connection&
	);

	void add_steps(
		wholth::entity::food::id_t,
		const std::span<const wholth::entity::editable::food::RecipeStep>,
		sqlw::Connection&,
		wholth::entity::locale::id_t
	);

	void update_steps(
		const std::span<const wholth::entity::editable::food::RecipeStep>,
		sqlw::Connection&,
		wholth::entity::locale::id_t
	);

	void remove_steps(
		const std::span<const wholth::entity::editable::food::RecipeStep>,
		sqlw::Connection&
	);

	void add_nutrients(
		wholth::entity::food::id_t,
		const std::span<const wholth::entity::editable::food::Nutrient>,
		sqlw::Connection&
	);

	void update_nutrients(
		wholth::entity::food::id_t,
		const std::span<const wholth::entity::editable::food::Nutrient>,
		sqlw::Connection&
	);

	void remove_nutrients(
		wholth::entity::food::id_t,
		const std::span<const wholth::entity::editable::food::Nutrient>,
		sqlw::Connection&
	);

	void add_ingredients(
		wholth::entity::recipe_step::id_t,
		const std::span<const wholth::entity::editable::food::Ingredient>,
		sqlw::Connection&
	);

	void update_ingredients(
		wholth::entity::recipe_step::id_t,
		const std::span<const wholth::entity::editable::food::Ingredient>,
		sqlw::Connection&
	);

	void remove_ingredients(
		wholth::entity::recipe_step::id_t,
		const std::span<const wholth::entity::editable::food::Ingredient>,
		sqlw::Connection&
	);

	struct FoodsQuery
	{
		uint32_t page {0};
		// @todo: programmaticlay resolve default locale_id.
		std::string_view locale_id {""};
		std::string_view title {""};
		std::span<nutrient_filter::Entry> nutrient_filters {};
		std::string_view food_names {""};
	};

	auto list_foods(
		std::span<wholth::entity::viewable::Food> list,
		std::string& buffer,
		const FoodsQuery&,
		sqlw::Connection*
	) -> PaginationInfo;
}

#endif // WHOLTH_ENTITY_FOOD_H_
