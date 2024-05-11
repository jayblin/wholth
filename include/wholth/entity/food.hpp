#ifndef WHOLTH_ENTITY_FOOD_H_
#define WHOLTH_ENTITY_FOOD_H_

#include <array>
#include <limits>
#include <optional>
#include <ostream>
#include <string>
#include <system_error>
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
#include "wholth/concepts.hpp"
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

namespace wholth::entity::shortened
{
	struct Food
	{
		wholth::entity::food::id_t id;
		wholth::entity::food::title_t title;
		std::string_view preparation_time;
		wholth::entity::nutrient::value_t top_nutrient;
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
	struct Food
	{
		wholth::entity::food::id_t id {wholth::utils::NIL};
		wholth::entity::food::title_t title {wholth::utils::NIL};
		wholth::entity::food::description_t description {wholth::utils::NIL};
	};

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
}

namespace wholth::entity::expanded
{
	struct Food
	{
		wholth::entity::food::id_t id;
		wholth::entity::food::title_t title;
		wholth::entity::food::description_t description;
		std::string_view preparation_time;
	};

	namespace food
	{
		struct Nutrient
		{
			wholth::entity::nutrient::id_t id;
			wholth::entity::nutrient::title_t title;
			wholth::entity::nutrient::value_t value;
			std::string_view unit;
			wholth::entity::nutrient::value_t user_value;
		};

		struct Ingredient
		{
			wholth::entity::food::id_t food_id;
			wholth::entity::food::title_t title;
			wholth::entity::ingredient::canonical_mass_t canonical_mass;
			std::string_view ingredient_count;
		};

		struct RecipeStep
		{
			wholth::entity::recipe_step::id_t id;
			wholth::entity::recipe_step::time_t time;
			wholth::entity::recipe_step::description_t description;
		};
	}
}

namespace wholth {

	enum class StatusCode : int
	{
		SQL_STATEMENT_ERROR = std::numeric_limits<int>::min(),
		NO_ERROR = 0,
		ENTITY_NOT_FOUND,
		INVALID_LOCALE_ID,
		INVALID_FOOD_ID,
		EMPTY_FOOD_TITLE,
		UNCHANGED_FOOD_TITLE,
		UNCHANGED_FOOD_DESCRIPTION,
	};

	std::string_view view(wholth::StatusCode rc);

	// @todo add tests for when couldn't create description and title.
	auto insert_food(
		const wholth::entity::editable::Food&,
		std::string& result_id,
		sqlw::Connection&,
		wholth::entity::locale::id_t
	) noexcept -> StatusCode;

	struct UpdateFoodStatus
	{
		StatusCode title {StatusCode::NO_ERROR};
		StatusCode description {StatusCode::NO_ERROR};
		StatusCode rc {StatusCode::NO_ERROR};
	};

	auto update_food(
		const wholth::entity::editable::Food&,
		sqlw::Connection&,
		wholth::entity::locale::id_t
	) noexcept -> UpdateFoodStatus;

	auto remove_food(
		wholth::entity::food::id_t,
		sqlw::Connection&
	) noexcept -> StatusCode;

	void add_steps(
		wholth::entity::food::id_t,
		const std::span<const wholth::entity::editable::food::RecipeStep>,
		sqlw::Connection&,
		wholth::entity::locale::id_t
	) noexcept;

	void update_steps(
		const std::span<const wholth::entity::editable::food::RecipeStep>,
		sqlw::Connection&,
		wholth::entity::locale::id_t
	) noexcept;

	void remove_steps(
		const std::span<const wholth::entity::editable::food::RecipeStep>,
		sqlw::Connection&
	) noexcept;

	void add_nutrients(
		wholth::entity::food::id_t,
		const std::span<const wholth::entity::editable::food::Nutrient>,
		sqlw::Connection&
	) noexcept;

	void update_nutrients(
		wholth::entity::food::id_t,
		const std::span<const wholth::entity::editable::food::Nutrient>,
		sqlw::Connection&
	) noexcept;

	void remove_nutrients(
		wholth::entity::food::id_t,
		const std::span<const wholth::entity::editable::food::Nutrient>,
		sqlw::Connection&
	) noexcept;

	void add_ingredients(
		wholth::entity::recipe_step::id_t,
		const std::span<const wholth::entity::editable::food::Ingredient>,
		sqlw::Connection&
	) noexcept;

	void update_ingredients(
		wholth::entity::recipe_step::id_t,
		const std::span<const wholth::entity::editable::food::Ingredient>,
		sqlw::Connection&
	) noexcept;

	void remove_ingredients(
		wholth::entity::recipe_step::id_t,
		const std::span<const wholth::entity::editable::food::Ingredient>,
		sqlw::Connection&
	) noexcept;

	/*struct FoodsQuery */
	/*{ */
	/*	uint32_t page {0}; */
	/*	std::string_view locale_id {""}; */
	/*	/1** */
	/*	 * Food's title. */
	/*	 *1/ */
	/*	std::string_view title {""}; */
	/*	/1** */
	/*	 * Coma separated list of ingredient names. */
	/*	 *1/ */
	/*	std::string_view ingredients {""}; */
	/*	std::span<nutrient_filter::Entry> nutrient_filters {}; */
	/*}; */

	/* auto list_foods( */
	/* 	std::span<wholth::entity::shortened::Food> list, */
	/* 	std::string& buffer, */
	/* 	const FoodsQuery&, */
	/* 	sqlw::Connection* */
	/* ) noexcept -> StatusCode; */

	auto expand_food(
		wholth::entity::expanded::Food&,
		std::string& buffer,
		wholth::entity::food::id_t,
		wholth::entity::locale::id_t,
		sqlw::Connection*
	) noexcept -> StatusCode;

	auto list_steps(
		std::span<wholth::entity::expanded::food::RecipeStep>,
		std::string& buffer,
		wholth::entity::food::id_t,
		wholth::entity::locale::id_t,
		sqlw::Connection*
	) noexcept -> StatusCode;

	auto list_ingredients(
		std::span<wholth::entity::expanded::food::Ingredient>,
		std::string& buffer,
		wholth::entity::food::id_t,
		wholth::entity::locale::id_t,
		sqlw::Connection*
	) noexcept -> StatusCode;

	auto list_nutrients(
		std::span<wholth::entity::expanded::food::Nutrient>,
		std::string& buffer,
		wholth::entity::food::id_t,
		wholth::entity::locale::id_t,
		sqlw::Connection*
	) noexcept -> StatusCode;

	auto describe_error(
		StatusCode ec,
		std::string& buffer,
		wholth::entity::locale::id_t locale_id,
		sqlw::Connection* con
	) noexcept -> StatusCode;

	 auto recalc_nutrients(
		wholth::entity::food::id_t,
		sqlw::Connection&
	) noexcept -> StatusCode;
}

// Return true if rc is an error.
constexpr bool operator!(wholth::StatusCode rc)
{
	return wholth::StatusCode::NO_ERROR != rc;
}

std::ostream& operator<<(std::ostream&, const wholth::entity::expanded::Food&);

#endif // WHOLTH_ENTITY_FOOD_H_
