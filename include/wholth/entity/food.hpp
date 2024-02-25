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
#include "wholth/pager.hpp"
#include "wholth/entity/utils.hpp"
#include "wholth/utils.hpp"

namespace wholth::entity::food
{
	typedef std::string_view id_t;
	typedef std::string_view title_t;
	typedef std::string_view description_t;

	/* namespace view */
	/* { */
	/* 	typedef TupleElement<std::string_view, 0> id; */
	/* 	typedef TupleElement<std::string_view, 1> title; */
	/* 	typedef TupleElement<std::string_view, 2> description; */
	/* }; */

	/* typedef std::tuple< */
	/* 	view::id::value_type, */
	/* 	view::title::value_type, */
	/* 	view::description::value_type */
	/* > View; */

	/* namespace input */
	/* { */
	/* 	/1* typedef TupleElement<TextInput<255>, 0> title; *1/ */
	/* 	/1* typedef TupleElement<FloatInput, 1> description; *1/ */
	/* 	/1* typedef TupleElement<std::string_view, 0> id; *1/ */
	/* 	/1* typedef TupleElement<std::string_view, 1> title; *1/ */
	/* 	/1* typedef TupleElement<std::string_view, 2> description; *1/ */
	/* 	/1* typedef TupleElement<std::span<wholth::entity::nutrient::, 2> description; *1/ */
	/* }; */

	/* typedef std::tuple< */
	/* 	input::title::value_type, */
	/* 	input::description::value_type */
	/* > Input; */
	/* namespace food_specific { */
	/* 	/1* typedef std::tuple< *1/ */
	/* 	/1* 	wholth::entity::nutrient::view::id::value_type, *1/ */
	/* 	/1* 	wholth::entity::nutrient::view::value::value_type *1/ */
	/* 	/1* > Nutrient; *1/ */
	/* 	struct Nutrient */
	/* 	{ */
	/* 		wholth::entity::nutrient::view::id::value_type id {wholth::utils::NIL}; */
	/* 		wholth::entity::nutrient::view::value::value_type value {wholth::utils::NIL}; */
	/* 	}; */
	/* } */

	/* namespace specific */
	/* { */
	/* 	struct ReicpeStep; */
	/* } */

	/* struct Input */
	/* { */
	/* 	/1* view::id::value_type id {""}; *1/ */
	/* 	/1* view::title::value_type title {wholth::utils::NIL}; *1/ */
	/* 	/1* view::description::value_type description {wholth::utils::NIL}; *1/ */
	/* 	id_t id {""}; */
	/* 	title_t title {wholth::utils::NIL}; */
	/* 	description_t description {wholth::utils::NIL}; */
	/* 	std::span<food_specific::Nutrient> nutrients; */
	/* 	std::span<specific::ReicpeStep> steps; */
	/* }; */

	/* namespace input */
	/* { */
	/* 	Input initialize(); */
	/* }; */

	/* void create( */
	/* 	sqlw::Connection&, */
	/* 	const wholth::entity::food::Input&, */
	/* 	const wholth::entity::locale::view::id::value_type& locale_id */
	/* ); */

	/* void update( */
	/* 	sqlw::Connection& con, */
	/* 	const Input& food, */
	/* 	const wholth::entity::locale::view::id::value_type locale_id */
	/* ) */
	/* { */
	/* 	std::size_t idx = 3; */
	/* 	std::stringstream ss2; */
	/* 	std::stringstream ss3; */

	/* 	if (food.title != wholth::utils::NIL) */
	/* 	{ */
	/* 		ss2 << "title,"; */
	/* 		ss3 << ":" << idx << ","; */
	/* 		idx++; */
	/* 	} */

	/* 	if (food.description != wholth::utils::NIL) */
	/* 	{ */
	/* 		ss2 << "description,"; */
	/* 		ss3 << ":" << idx << ","; */
	/* 		idx++; */
	/* 	} */

	/* 	/1* std::string res = ss2.str(); *1/ */

	/* 	size_t ss2_len = ss2.rdbuf()->in_avail(); */
	/* 	size_t ss3_len = ss3.rdbuf()->in_avail(); */

	/* 	if (ss2_len == 0) */
	/* 	{ */
	/* 		return; */
	/* 	} */

	/* 	sqlw::Statement stmt {&con}; */

	/* 	/1* wholth::utils::IndexSequence<1> idx; *1/ */

	/* 	std::string sql = fmt::format( */
	/* 		"INSERT OR REPLACE " */
	/* 		"INTO food_localisation (food_id,locale_id,{}) " */
	/* 		"VALUES (:1,:2,{})", */
	/* 		ss2.str().substr(0, ss2_len - 1), */
	/* 		ss3.str().substr(0, ss3_len - 1) */
	/* 	); */
	/* 		/1* "INTO food_localisation (food_id,locale_id,title,description) " *1/ */
	/* 		/1* "VALUES (:1,:2,COALESCE(:3,title),COALESCE(:4,description)) "; *1/ */
	/* 		/1* "ON CONFLICT "; *1/ */

	/* 	/1* stmt(ss.str()); *1/ */
	/* 	stmt(sql); */

	/* 	if (sqlw::status::is_ok(stmt.status())) */
	/* 	{ */
	/* 		// idk */
	/* 	} */

	/* 	for (const wholth::entity::food::food_specific::Nutrient& nutrient : food.nutrients) */
	/* 	{ */
	/* 		if (nutrient.id == wholth::utils::NIL || nutrient.value == wholth::utils::NIL) */
	/* 		{ */
	/* 		} */
	/* 	} */
	/* } */
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

namespace wholth::entity::recipe_step_food
{
	/* typedef wholth::entity::food::id_t food_id; */
	typedef std::string_view canonical_mass_t;
};

namespace wholth::entity::recipe_step
{
	/* typedef wholth::entity::food::id_t food_id; */
	typedef std::string_view id_t;
	typedef std::string_view time_t;
	typedef std::string_view note_t;
	typedef std::string_view description_t;
	/* typedef std::string_view canonical_mass_t; */
};

namespace wholth::entity::viewable
{
	struct Food
	{
		wholth::entity::food::id_t id;
		wholth::entity::food::title_t title;
		std::string_view preparation_time;
		/* std::string_view health_index; */
		/* wholth::entity::food::description_t description; */
	};

	namespace expanded
	{
		struct Food
		{
			wholth::entity::food::id_t id;
			wholth::entity::food::title_t title;
			std::string_view preparation_time;
			/* std::string_view health_index; */
			wholth::entity::food::description_t description;
		};
	};

	/* namespace food */
	/* { */
	/* 	struct Nutrient */
	/* 	{ */
	/* 		wholth::entity::nutrient::id_t id {wholth::utils::NIL}; */
	/* 		wholth::entity::nutrient::value_t value {wholth::utils::NIL}; */
	/* 	}; */

	/* 	struct RecipeStep */
	/* 	{ */
	/* 		wholth::entity::recipe_step::id_t id {wholth::utils::NIL}; */
	/* 		wholth::entity::recipe_step::time_t time {wholth::utils::NIL}; */
	/* 		wholth::entity::recipe_step::description_t description {wholth::utils::NIL}; */
	/* 		wholth::entity::recipe_step::note_t note {wholth::utils::NIL}; */
	/* 	}; */
	/* }; */
	
}

namespace wholth::entity::editable
{
	/* struct Nutrient */
	/* { */
	/* 	wholth::entity::nutrient::id_t id {wholth::utils::NIL}; */
	/* 	wholth::entity::nutrient::title_t title {wholth::utils::NIL}; */
	/* 	wholth::entity::nutrient::value_t value {wholth::utils::NIL}; */
	/* 	wholth::entity::nutrient::unit_t unit {wholth::utils::NIL}; */
	/* 	wholth::entity::nutrient::alias_t alias {wholth::utils::NIL}; */
	/* 	wholth::entity::nutrient::description_t description {wholth::utils::NIL}; */
	/* }; */

	/* struct RecipeStepFood */
	/* { */
	/* 	wholth::entity::recipe_step::id_t recipe_step_id {wholth::utils::NIL}; */
	/* 	wholth::entity::food::id_t food_id {wholth::utils::NIL}; */
	/* 	wholth::entity::recipe_step_food::canonical_mass_t canonical_mass {wholth::utils::NIL}; */
	/* }; */

	/* struct RecipeStep */
	/* { */
	/* 	wholth::entity::recipe_step::id_t id {wholth::utils::NIL}; */
	/* 	wholth::entity::food::id_t recipe_id {wholth::utils::NIL}; */
	/* 	wholth::entity::recipe_step::time_t time {wholth::utils::NIL}; */
	/* 	wholth::entity::recipe_step::description_t description {wholth::utils::NIL}; */
	/* 	wholth::entity::recipe_step::note_t note {wholth::utils::NIL}; */
	/* }; */

	namespace food
	{
		struct Nutrient
		{
			wholth::entity::nutrient::id_t id {wholth::utils::NIL};
			wholth::entity::nutrient::value_t value {wholth::utils::NIL};
		};

		struct RecipeStepFood
		{
			wholth::entity::food::id_t food_id {wholth::utils::NIL};
			wholth::entity::recipe_step_food::canonical_mass_t canonical_mass {wholth::utils::NIL};
		};

		struct RecipeStep
		{
			wholth::entity::recipe_step::id_t id {wholth::utils::NIL};
			wholth::entity::recipe_step::time_t time {wholth::utils::NIL};
			wholth::entity::recipe_step::description_t description {wholth::utils::NIL};
			std::span<RecipeStepFood> foods;
		};
	};

	struct Food
	{
		wholth::entity::food::id_t id {wholth::utils::NIL};
		wholth::entity::food::title_t title {wholth::utils::NIL};
		std::string_view preparation_time {wholth::utils::NIL};
		wholth::entity::food::description_t description {wholth::utils::NIL};
		std::span<wholth::entity::editable::food::Nutrient> nutrients;
		std::span<wholth::entity::editable::food::RecipeStep> steps;
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

	/* template<> template<> */
	/* PaginationInfo Pager<entity::food::View>::query_page<FoodsQuery>( */
	/* 	std::span<entity::food::View>, */
	/* 	sqlw::Connection*, */
	/* 	const FoodsQuery& */
	/* ); */

	template<> template<>
	PaginationInfo Pager<entity::viewable::Food>::query_page<FoodsQuery>(
		std::span<entity::viewable::Food>,
		sqlw::Connection*,
		const FoodsQuery&
	);

	void insert(
		sqlw::Connection&,
		/* const wholth::entity::food::Input&, */
		const wholth::entity::editable::Food& food,
		const wholth::entity::locale::view::id::value_type& locale_id
	);

}

/* namespace wholth::entity::recipe_step */
/* { */
/* 	namespace view */
/* 	{ */
/* 		typedef TupleElement<std::string_view, 0> id; */
/* 		typedef TupleElement<wholth::entity::food::view::id::value_type, 1> recipe_id; */
/* 		typedef TupleElement<std::string_view, 2> cooking_action_id; */
/* 		typedef TupleElement<std::string_view, 3> priority; */
/* 		typedef TupleElement<std::string_view, 4> time; */
/* 		typedef TupleElement<std::string_view, 5> description; */
/* 		typedef TupleElement<std::string_view, 6> note; */
/* 	}; */
/* } */

/* namespace wholth::entity::recipe_step_food */
/* { */
/* 	namespace view */
/* 	{ */
/* 		typedef TupleElement<std::string_view, 0> recipe_step_id; */
/* 		typedef TupleElement<wholth::entity::food::view::id::value_type, 1> food_id; */
/* 		typedef TupleElement<std::string_view, 2> canonical_mass; */
/* 	}; */
/* } */

/* namespace wholth::entity::food::specific */
/* { */
/* 	struct RecipeStepFood */
/* 	{ */
/* 		wholth::entity::recipe_step_food::view::food_id::value_type food_id; */
/* 		wholth::entity::recipe_step_food::view::canonical_mass::value_type canonical_mass; */
/* 	}; */

/* 	struct RecipeStep */
/* 	{ */
/* 		wholth::entity::recipe_step::view::time::value_type time; */
/* 		wholth::entity::recipe_step::view::description::value_type description; */
/* 		std::span<RecipeStepFood> foods; */
/* 	}; */
/* } */

#endif // WHOLTH_ENTITY_FOOD_H_
