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
#include "utils/serializer.hpp"
#include "wholth/concepts.hpp"
#include "wholth/entity/locale.hpp"
#include "wholth/entity/nutrient.hpp"
/* #include "wholth/pager.hpp" */
#include "wholth/entity/utils.hpp"
#include "wholth/pager.hpp"
#include "wholth/status.hpp"
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

namespace wholth::entity::consumption_log
{
	typedef std::string_view mass_t;
	typedef double mass_numeric_t;
	typedef std::string_view consumed_at_t;
};

namespace wholth::entity::shortened
{
	struct Food
	{
		wholth::entity::food::id_t id;
		wholth::entity::food::title_t title;
		std::string_view preparation_time;
		wholth::entity::nutrient::value_t top_nutrient;

        template <typename Serializer>
        auto serialize(Serializer& serializer) const noexcept -> void
        {
            serializer
                << NVP(id)
                << NVP(title)
                << NVP(preparation_time)
                << NVP(top_nutrient)
            ;
        }
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
		/* char id[255] {wholth::utils::NIL}; */
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

        template <typename Serializer>
        auto serialize(Serializer& serializer) const noexcept -> void
        {
            serializer
                << NVP(id)
                << NVP(title)
                << NVP(description)
                << NVP(preparation_time)
            ;
        }
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

            template <typename Serializer>
            auto serialize(Serializer& serializer) const noexcept -> void
            {
                serializer
                    << NVP(id)
                    << NVP(title)
                    << NVP(value)
                    << NVP(unit)
                    << NVP(user_value)
                ;
            }
		};

		struct Ingredient
		{
			wholth::entity::food::id_t food_id;
			wholth::entity::food::title_t title;
			wholth::entity::ingredient::canonical_mass_t canonical_mass;
			std::string_view ingredient_count;

            template <typename Serializer>
            auto serialize(Serializer& serializer) const noexcept -> void
            {
                serializer
                    << NVP(food_id)
                    << NVP(title)
                    << NVP(canonical_mass)
                    << NVP(ingredient_count)
                ;
            }
		};

		struct RecipeStep
		{
			wholth::entity::recipe_step::id_t id;
			wholth::entity::recipe_step::time_t time;
			wholth::entity::recipe_step::description_t description;

            template <typename Serializer>
            auto serialize(Serializer& serializer) const noexcept -> void
            {
                serializer
                    << NVP(id)
                    << NVP(time)
                    << NVP(description)
                ;
            }
		};
	}
}

namespace wholth {
	// @todo add tests for when couldn't create description and title.
    [[nodiscard]]
	auto insert_food(
		const wholth::entity::editable::Food&,
		const wholth::entity::locale::id_t,
		std::string& result_id,
		sqlw::Connection&
	) noexcept -> std::error_code;

	struct UpdateFoodStatus
	{
        std::error_code title {wholth::status::Code::OK};
        std::error_code description {wholth::status::Code::OK};
        std::error_code ec {wholth::status::Code::OK};
	};
    
    struct SpanResult
    {
        size_t success_count;
        std::error_code error_code;
    };

    [[nodiscard]]
	auto update_food(
		const wholth::entity::editable::Food&,
		wholth::entity::locale::id_t,
		sqlw::Connection&
	) noexcept -> UpdateFoodStatus;

    [[nodiscard]]
	auto remove_food(
		wholth::entity::food::id_t,
		sqlw::Connection&
	) noexcept -> std::error_code;

    /* [[nodiscard]] */
	/* auto add_steps( */
		/* wholth::entity::food::id_t, */
		/* wholth::entity::locale::id_t, */
		/* std::span<const wholth::entity::editable::food::RecipeStep>, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto update_steps( */
		/* std::span<const wholth::entity::editable::food::RecipeStep>, */
		/* wholth::entity::locale::id_t, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto remove_steps( */
		/* std::span<const wholth::entity::editable::food::RecipeStep>, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto add_nutrients( */
		/* wholth::entity::food::id_t, */
		/* std::span<const wholth::entity::editable::food::Nutrient>, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto update_nutrients( */
		/* wholth::entity::food::id_t, */
		/* std::span<const wholth::entity::editable::food::Nutrient>, */
		/* sqlw::Connection& */
	/* ) noexcept -> SpanResult; */

    /* [[nodiscard]] */
	/* auto remove_nutrients( */
		/* wholth::entity::food::id_t, */
		/* std::span<const wholth::entity::editable::food::Nutrient>, */
		/* sqlw::Connection& */
	/* ) noexcept -> SpanResult; */

    /* [[nodiscard]] */
	/* auto add_ingredients( */
		/* wholth::entity::recipe_step::id_t, */
		/* std::span<const wholth::entity::editable::food::Ingredient>, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto update_ingredients( */
		/* wholth::entity::recipe_step::id_t, */
		/* std::span<const wholth::entity::editable::food::Ingredient>, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto remove_ingredients( */
		/* wholth::entity::recipe_step::id_t, */
		/* std::span<const wholth::entity::editable::food::Ingredient>, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

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
	/* ) noexcept -> wholth::status::Code; */

    /* [[nodiscard]] */
	/* auto expand_food( */
		/* wholth::entity::expanded::Food&, */
		/* wholth::entity::food::id_t, */
		/* wholth::entity::locale::id_t, */
		/* std::string& buffer, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto list_steps( */
		/* std::span<wholth::entity::expanded::food::RecipeStep>, */
		/* wholth::entity::food::id_t, */
		/* wholth::entity::locale::id_t, */
		/* std::string& buffer, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto list_ingredients( */
		/* std::span<wholth::entity::expanded::food::Ingredient>, */
		/* wholth::entity::food::id_t, */
		/* wholth::entity::locale::id_t, */
		/* std::string& buffer, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto list_nutrients( */
		/* std::span<wholth::entity::expanded::food::Nutrient>, */
		/* wholth::entity::food::id_t, */
		/* wholth::entity::locale::id_t, */
		/* std::string& buffer, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto describe_error( */
		/* wholth::status::Code ec, */
		/* wholth::entity::locale::id_t locale_id, */
		/* std::string& buffer, */
		/* sqlw::Connection& con */
	/* ) noexcept -> std::error_code; */

	/* // todo test return codes. */
    /* [[nodiscard]] */
	/* auto recalc_nutrients( */
		/* wholth::entity::food::id_t, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

	/* // todo */
	/* // - check date check */
	/* // - check date subst */
    /* [[nodiscard]] */
	/* auto log_consumption( */
		/* wholth::entity::food::id_t, */
		/* wholth::entity::consumption_log::mass_t, */
		/* wholth::entity::consumption_log::consumed_at_t, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */

    /* [[nodiscard]] */
	/* auto log_consumption( */
		/* wholth::entity::food::id_t, */
		/* wholth::entity::consumption_log::mass_numeric_t, */
		/* wholth::entity::consumption_log::consumed_at_t, */
		/* sqlw::Connection& */
	/* ) noexcept -> std::error_code; */
}

#endif // WHOLTH_ENTITY_FOOD_H_
