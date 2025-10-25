#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <limits>
#include <type_traits>
#include "helpers.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/context.hpp"
#include "wholth/controller/abstract_page.hpp"
#include "wholth/model/expanded_food.hpp"
#include "wholth/status.hpp"
#include "wholth/entity/food.hpp"

class ListIngredientsTest : public MigrationAwareTest
{
  protected:
    static void SetUpTestSuite()
    {
        MigrationAwareTest::SetUpTestSuite();
        sqlw::Statement stmt{&db_con};

        auto ec = stmt("INSERT INTO food (id, created_at) "
                       "VALUES "
                       " (1,'10-10-2010'),"
                       " (2,'10-10-2010'),"
                       " (3,'10-10-2010'),"
                       " (4,'10-10-2010'),"
                       " (5,'10-10-2010'),"
                       " (6,'10-10-2010'),"
                       " (7,'10-10-2010')");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
        ec = stmt("INSERT INTO locale (id,alias) VALUES "
                  "(1,'EN'),(2,'RU'),(3,'DE')");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
        ec = stmt(
            "INSERT INTO food_localisation (food_id,locale_id,title) VALUES "
            " (1,2,'food_1_ru'),"
            " (2,2,'food_2_ru'),"
            " (3,2,'food_3_ru'),"
            " (4,2,'food_4_ru'),"
            " (5,1,'food_5_en'),"
            " (6,2,'food_6_ru'),"
            " (7,2,'food_7_ru')");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
        ec = stmt("INSERT INTO recipe_step (id,recipe_id) "
                  "VALUES "
                  "(1, 1),"
                  "(2, 1),"
                  "(3, 2),"
                  "(4, 5)");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
        ec = stmt("INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) "
                  "VALUES "
                  "(1, 3, 130),"
                  "(1, 4, 140),"
                  "(2, 5, 150),"
                  "(3, 5, 151),"
                  "(4, 7, 170)");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
    }
};

// No locale in query
TEST_F(ListIngredientsTest, when_no_lcoale_in_query)
{
    wholth::Context ctx{};
    ctx.locale_id = "";

    wholth::model::FoodIngredientsContainer<wholth::entity::Ingredient> container {};
    wholth::model::FoodIngredients q{
        .ctx = ctx,
        .pagination={container.size}
    };
    auto ec = wholth::controller::fill_container_through_model(container, q, db_con);
    auto list = container.swappable_buffer_views.view_current().view;

    ASSERT_TRUE(wholth::status::Code::INVALID_LOCALE_ID == ec) << ec;
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
    ASSERT_STREQ2("", list[0].food_id);
    ASSERT_STREQ2("", list[0].food_title);
    ASSERT_STREQ2("", list[0].canonical_mass_g);
    ASSERT_STREQ2("", list[0].ingredient_count);
    ASSERT_STREQ2("", list[1].food_id);
    ASSERT_STREQ2("", list[1].food_title);
    ASSERT_STREQ2("", list[1].canonical_mass_g);
    ASSERT_STREQ2("", list[1].ingredient_count);
}

// No food id in query
TEST_F(ListIngredientsTest, when_empty_food_id)
{
    wholth::Context ctx{};
    ctx.locale_id = "2";

    wholth::model::FoodIngredientsContainer<wholth::entity::Ingredient> container {};
    wholth::model::FoodIngredients q{
        .ctx = ctx,
        .pagination={container.size}
    };
    auto ec = wholth::controller::fill_container_through_model(container, q, db_con);
    auto list = container.swappable_buffer_views.view_current().view;

    ASSERT_TRUE(wholth::status::Code::INVALID_FOOD_ID == ec) << ec;
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
    ASSERT_STREQ2("", list[0].food_id);
    ASSERT_STREQ2("", list[0].food_title);
    ASSERT_STREQ2("", list[0].canonical_mass_g);
    ASSERT_STREQ2("", list[0].ingredient_count);
    ASSERT_STREQ2("", list[1].food_id);
    ASSERT_STREQ2("", list[1].food_title);
    ASSERT_STREQ2("", list[1].canonical_mass_g);
    ASSERT_STREQ2("", list[1].ingredient_count);
}

TEST_F(ListIngredientsTest, when_invalid_locale_in_query)
{
    wholth::Context ctx{};
    ctx.locale_id = "2yy";

    wholth::model::FoodIngredientsContainer<wholth::entity::Ingredient> container {};
    wholth::model::FoodIngredients q{
        .ctx = ctx,
        .pagination={container.size}
    };
    auto ec = wholth::controller::fill_container_through_model(container, q, db_con);
    auto list = container.swappable_buffer_views.view_current().view;

    ASSERT_TRUE(wholth::status::Code::INVALID_LOCALE_ID == ec) << ec;
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
    ASSERT_STREQ2("", list[0].food_id);
    ASSERT_STREQ2("", list[0].food_title);
    ASSERT_STREQ2("", list[0].canonical_mass_g);
    ASSERT_STREQ2("", list[0].ingredient_count);
    ASSERT_STREQ2("", list[1].food_id);
    ASSERT_STREQ2("", list[1].food_title);
    ASSERT_STREQ2("", list[1].canonical_mass_g);
    ASSERT_STREQ2("", list[1].ingredient_count);
}

// Inlvalid id in query
TEST_F(ListIngredientsTest, when_invalid_food_id)
{
    wholth::Context ctx{};
    ctx.locale_id = "2";

    wholth::model::FoodIngredientsContainer<wholth::entity::Ingredient> container {};
    wholth::model::FoodIngredients q{
        .ctx = ctx,
        .pagination={container.size},
        .food_id="3e",
    };
    auto ec = wholth::controller::fill_container_through_model(container, q, db_con);
    auto list = container.swappable_buffer_views.view_current().view;

    ASSERT_TRUE(wholth::status::Code::INVALID_FOOD_ID == ec) << ec;
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
    ASSERT_STREQ2("", list[0].food_id);
    ASSERT_STREQ2("", list[0].food_title);
    ASSERT_STREQ2("", list[0].canonical_mass_g);
    ASSERT_STREQ2("", list[0].ingredient_count);
    ASSERT_STREQ2("", list[1].food_id);
    ASSERT_STREQ2("", list[1].food_title);
    ASSERT_STREQ2("", list[1].canonical_mass_g);
    ASSERT_STREQ2("", list[1].ingredient_count);
}

// Requested page number is too big for int.
TEST_F(ListIngredientsTest, when_requested_page_number_is_to_big)
{
    wholth::Context ctx{};
    ctx.locale_id = "2";

    wholth::model::FoodIngredientsContainer<wholth::entity::Ingredient> container {};
    wholth::model::FoodIngredients q{
        .ctx = ctx,
        .pagination={container.size},
        .food_id="3",
    };
    wholth::controller::skip_to(q, std::numeric_limits<uint64_t>::max());

    auto ec = wholth::controller::fill_container_through_model(container, q, db_con);
    auto list = container.swappable_buffer_views.view_current().view;

    ASSERT_TRUE(wholth::status::Code::QUERY_PAGE_TOO_BIG == ec) << ec;
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
    ASSERT_STREQ2("", list[0].food_id);
    ASSERT_STREQ2("", list[0].food_title);
    ASSERT_STREQ2("", list[0].canonical_mass_g);
    ASSERT_STREQ2("", list[0].ingredient_count);
    ASSERT_STREQ2("", list[1].food_id);
    ASSERT_STREQ2("", list[1].food_title);
    ASSERT_STREQ2("", list[1].canonical_mass_g);
    ASSERT_STREQ2("", list[1].ingredient_count);
}

// Requested offset (page*span.size) is too big for int.
TEST_F(ListIngredientsTest, when_rquested_offset_is_to_big)
{
    wholth::Context ctx{};
    ctx.locale_id = "2";

    wholth::model::FoodIngredientsContainer<wholth::entity::Ingredient> container {};
    wholth::model::FoodIngredients q{
        .ctx = ctx,
        .pagination={container.size},
        .food_id="3",
    };
    wholth::controller::skip_to(q, std::numeric_limits<uint64_t>::max() / 2);

    auto ec = wholth::controller::fill_container_through_model(container, q, db_con);
    auto list = container.swappable_buffer_views.view_current().view;

    ASSERT_TRUE(wholth::status::Code::QUERY_PAGE_TOO_BIG == ec) << ec;
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
    ASSERT_STREQ2("", list[0].food_id);
    ASSERT_STREQ2("", list[0].food_title);
    ASSERT_STREQ2("", list[0].canonical_mass_g);
    ASSERT_STREQ2("", list[0].ingredient_count);
    ASSERT_STREQ2("", list[1].food_id);
    ASSERT_STREQ2("", list[1].food_title);
    ASSERT_STREQ2("", list[1].canonical_mass_g);
    ASSERT_STREQ2("", list[1].ingredient_count);
}

// Basic use case.
TEST_F(ListIngredientsTest, when_basic_case)
{
    wholth::Context ctx{};
    ctx.locale_id = "2";

    wholth::model::FoodIngredientsContainer<wholth::entity::Ingredient> container {};
    wholth::model::FoodIngredients q{
        .ctx = ctx,
        .pagination={container.size},
        .food_id="1",
    };

    auto ec = wholth::controller::fill_container_through_model(container, q, db_con);
    auto list = container.swappable_buffer_views.view_current().view;

    ASSERT_TRUE(wholth::status::Code::OK == ec) << ec;
    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2("3", list[0].food_id);
    ASSERT_STREQ2("food_3_ru", list[0].food_title);
    ASSERT_STREQ2("130", list[0].canonical_mass_g);
    ASSERT_STREQ2("0", list[0].ingredient_count);
    ASSERT_STREQ2("4", list[1].food_id);
    ASSERT_STREQ2("food_4_ru", list[1].food_title);
    ASSERT_STREQ2("140", list[1].canonical_mass_g);
    ASSERT_STREQ2("0", list[1].ingredient_count);
    ASSERT_STREQ2("5", list[2].food_id);
    ASSERT_STREQ2("[N/A]", list[2].food_title);
    ASSERT_STREQ2("150", list[2].canonical_mass_g);
    ASSERT_STREQ2("1", list[2].ingredient_count);
    ASSERT_STREQ2("", list[3].food_id);
    ASSERT_STREQ2("", list[3].food_title);
    ASSERT_STREQ2("", list[3].canonical_mass_g);
    ASSERT_STREQ2("", list[3].ingredient_count);
    ASSERT_EQ(3, q.pagination.count()) << "Total count of ingredients for specified food";
}

// Basic use case. Span size is smaller than total element count.
TEST_F(ListIngredientsTest, when_span_size_is_larget_than_total)
{
    wholth::Context ctx{};
    ctx.locale_id = "2";

    wholth::model::FoodIngredientsContainer<wholth::entity::Ingredient, 2> container {};
    wholth::model::FoodIngredients q{
        .ctx = ctx,
        .pagination={container.size},
        .food_id="1",
    };

    auto ec = wholth::controller::fill_container_through_model(container, q, db_con);
    auto list = container.swappable_buffer_views.view_current().view;

    ASSERT_TRUE(wholth::status::Code::OK == ec) << ec;
    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2("3", list[0].food_id);
    ASSERT_STREQ2("food_3_ru", list[0].food_title);
    ASSERT_STREQ2("130", list[0].canonical_mass_g);
    ASSERT_STREQ2("0", list[0].ingredient_count);
    ASSERT_STREQ2("4", list[1].food_id);
    ASSERT_STREQ2("food_4_ru", list[1].food_title);
    ASSERT_STREQ2("140", list[1].canonical_mass_g);
    ASSERT_STREQ2("0", list[1].ingredient_count);
    ASSERT_EQ(3, q.pagination.count()) << "Total count of ingredients for specified food";
    ASSERT_EQ(1, q.pagination.max_page()) << "Total count of pages for specified food";

    wholth::controller::advance(q, 1);
    ec = wholth::controller::fill_container_through_model(container, q, db_con);
    list = container.swappable_buffer_views.view_current().view;

    ASSERT_TRUE(wholth::status::Code::OK == ec) << ec;
    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2("5", list[0].food_id);
    ASSERT_STREQ2("[N/A]", list[0].food_title);
    ASSERT_STREQ2("150", list[0].canonical_mass_g);
    ASSERT_STREQ2("1", list[0].ingredient_count);
    ASSERT_STREQ2("", list[1].food_id);
    ASSERT_STREQ2("", list[1].food_title);
    ASSERT_STREQ2("", list[1].canonical_mass_g);
    ASSERT_STREQ2("", list[1].ingredient_count);
    ASSERT_EQ(3, q.pagination.count()) << "Total count of ingredients for specified food";
    ASSERT_EQ(1, q.pagination.max_page()) << "Total count of pages for specified food";
}
