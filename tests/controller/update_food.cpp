
#include "wholth/controller/update_food.hpp"
#include "gtest/gtest.h"
#include "helpers.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/context.hpp"
#include "wholth/entity/food.hpp"
#include "wholth/status.hpp"
#include <gtest/gtest.h>
#include <type_traits>

class UpdateFoodTest : public MigrationAwareTest
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
        ec = stmt("INSERT INTO food_localisation "
                  "(food_id,locale_id,title,description) VALUES "
                  " (1,2,'food_1_ru','description of food 1'),"
                  " (2,2,'food_2_ru','description of food 2'),"
                  " (3,2,'food_3_ru','description of food 3'),"
                  " (4,2,'food_4_ru','description of food 4'),"
                  " (5,1,'food_5_en','description of food 5'),"
                  " (6,2,'food_6_ru','description of food 6'),"
                  " (7,2,'food_7_ru','description of food 7')");
    }
};

TEST_F(UpdateFoodTest, when_food_exists_but_not_localised)
{
    sqlw::Statement stmt{&db_con};

    // Title and description are updated only for the specified food.
    wholth::entity::Food food{
        .id = "4",
        .title = "  Salt ",
    };
    wholth::entity::food::Details deets{
        .description = "for nerves",
    };
    wholth::Context ctx{};
    ctx.locale_id = "3";
    auto ec = wholth::controller::update(food, db_con, ctx, deets);
    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec << ec.message();

    std::vector<std::string> titles{};
    std::vector<std::string> descriptions{};
    ec = stmt(
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 4 "
        "ORDER BY locale_id ASC",
        [&](auto e) {
            if (e.column_name == "title")
            {
                titles.emplace_back(e.column_value);
            }
            else
            {
                descriptions.emplace_back(e.column_value);
            }
        });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_EQ(2, titles.size());
    ASSERT_EQ(2, descriptions.size());
    ASSERT_STREQ2("food_4_ru", titles[0]);
    ASSERT_STREQ2("salt", titles[1]) << "the title should be trimmed also";
    ASSERT_STREQ2("description of food 4", descriptions[0]);
    ASSERT_STREQ2("for nerves", descriptions[1]);
}

TEST_F(UpdateFoodTest, when_food_exists_and_localised)
{
    sqlw::Statement stmt{&db_con};

    wholth::entity::Food food{
        .id = "4",
        .title = "  Salt ",
    };
    wholth::entity::food::Details deets{
        .description = "for nerves",
    };
    wholth::Context ctx{};
    ctx.locale_id = "2";
    auto ec = wholth::controller::update(food, db_con, ctx, deets);
    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec << ec.message();

    std::vector<std::string> titles{};
    std::vector<std::string> descriptions{};
    ec = stmt(
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 4 "
        "ORDER BY locale_id ASC",
        [&](auto e) {
            if (e.column_name == "title")
            {
                titles.emplace_back(e.column_value);
            }
            else
            {
                descriptions.emplace_back(e.column_value);
            }
        });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_EQ(1, titles.size());
    ASSERT_EQ(1, descriptions.size());
    ASSERT_STREQ2("salt", titles[0]) << "the title should be trimmed also";
    ASSERT_STREQ2("for nerves", descriptions[0]);
}

TEST_F(UpdateFoodTest, when_food_does_not_exist)
{
    sqlw::Statement stmt{&db_con};

    wholth::entity::Food food{
        .id = "23334",
        .title = "  Salt ",
    };
    wholth::entity::food::Details deets{
        .description = "for nerves",
    };
    wholth::Context ctx{};
    ctx.locale_id = "2";
    auto ec = wholth::controller::update(food, db_con, ctx, deets);
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec << ec.message();

    std::vector<std::string> titles{};
    std::vector<std::string> descriptions{};
    ec = stmt(
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 23334 "
        "ORDER BY locale_id ASC",
        [&](auto e) {
            if (e.column_name == "title")
            {
                titles.emplace_back(e.column_value);
            }
            else
            {
                descriptions.emplace_back(e.column_value);
            }
        });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_EQ(0, titles.size());
    ASSERT_EQ(0, descriptions.size());
}

TEST_F(UpdateFoodTest, when_no_locale_id)
{
    sqlw::Statement stmt{&db_con};

    wholth::entity::Food food{
        .id = "4",
        .title = "  Salt ",
    };
    wholth::entity::food::Details deets{
        .description = "for nerves",
    };
    wholth::Context ctx{};
    ctx.locale_id = "";
    auto ec = wholth::controller::update(food, db_con, ctx, deets);
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec << ec.message();
    ASSERT_TRUE(wholth::status::Code::INVALID_LOCALE_ID == ec)
        << ec << ec.message();

    std::vector<std::string> titles{};
    std::vector<std::string> descriptions{};
    ec = stmt(
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 23334 "
        "ORDER BY locale_id ASC",
        [&](auto e) {
            if (e.column_name == "title")
            {
                titles.emplace_back(e.column_value);
            }
            else
            {
                descriptions.emplace_back(e.column_value);
            }
        });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_EQ(0, titles.size());
    ASSERT_EQ(0, descriptions.size());
}

TEST_F(UpdateFoodTest, when_bad_locale_id)
{
    sqlw::Statement stmt{&db_con};

    wholth::entity::Food food{
        .id = "4",
        .title = "  Salt ",
    };
    wholth::entity::food::Details deets{
        .description = "for nerves",
    };
    wholth::Context ctx{};
    ctx.locale_id = "3-f";
    auto ec = wholth::controller::update(food, db_con, ctx, deets);
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec << ec.message();
    ASSERT_TRUE(wholth::status::Code::INVALID_LOCALE_ID == ec)
        << ec << ec.message();

    std::vector<std::string> titles{};
    std::vector<std::string> descriptions{};
    ec = stmt(
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 23334 "
        "ORDER BY locale_id ASC",
        [&](auto e) {
            if (e.column_name == "title")
            {
                titles.emplace_back(e.column_value);
            }
            else
            {
                descriptions.emplace_back(e.column_value);
            }
        });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_EQ(0, titles.size());
    ASSERT_EQ(0, descriptions.size());
}

TEST_F(UpdateFoodTest, when_no_food_id)
{
    sqlw::Statement stmt{&db_con};

    wholth::entity::Food food{
        .id = "",
        .title = "  Salt ",
    };
    wholth::entity::food::Details deets{
        .description = "for nerves",
    };
    wholth::Context ctx{};
    ctx.locale_id = "2";
    auto ec = wholth::controller::update(food, db_con, ctx, deets);
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec << ec.message();
    ASSERT_TRUE(wholth::status::Code::INVALID_FOOD_ID == ec)
        << ec << ec.message();

    std::vector<std::string> titles{};
    std::vector<std::string> descriptions{};
    ec = stmt(
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 23334 "
        "ORDER BY locale_id ASC",
        [&](auto e) {
            if (e.column_name == "title")
            {
                titles.emplace_back(e.column_value);
            }
            else
            {
                descriptions.emplace_back(e.column_value);
            }
        });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_EQ(0, titles.size());
    ASSERT_EQ(0, descriptions.size());
}

TEST_F(UpdateFoodTest, when_bad_food_id)
{
    sqlw::Statement stmt{&db_con};

    wholth::entity::Food food{
        .id = "-4",
        .title = "  Salt ",
    };
    wholth::entity::food::Details deets{
        .description = "for nerves",
    };
    wholth::Context ctx{};
    ctx.locale_id = "2";
    auto ec = wholth::controller::update(food, db_con, ctx, deets);
    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec << ec.message();
    ASSERT_TRUE(wholth::status::Code::INVALID_FOOD_ID == ec)
        << ec << ec.message();

    std::vector<std::string> titles{};
    std::vector<std::string> descriptions{};
    ec = stmt(
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 23334 "
        "ORDER BY locale_id ASC",
        [&](auto e) {
            if (e.column_name == "title")
            {
                titles.emplace_back(e.column_value);
            }
            else
            {
                descriptions.emplace_back(e.column_value);
            }
        });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_EQ(0, titles.size());
    ASSERT_EQ(0, descriptions.size());
}
