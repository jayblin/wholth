#include <gtest/gtest.h>
#include <sstream>
#include <type_traits>
#include "helpers.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/context.hpp"
#include "wholth/controller/insert_recipe_step.hpp"
#include "wholth/status.hpp"
#include "wholth/entity/food.hpp"

class InsertRecipeStepTest : public MigrationAwareTest
{
  protected:
    static void SetUpTestSuite()
    {
        MigrationAwareTest::SetUpTestSuite();
        sqlw::Statement stmt{&db_con};

        auto ec = stmt("INSERT INTO food (id, created_at) "
                       "VALUES "
                       " (1,'10-10-2010'),"
                       " (2,'10-10-2010')");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
        ec = stmt("INSERT INTO locale (id,alias) VALUES "
                  "(1,'EN'),(2,'RU'),(3,'DE')");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
        ec = stmt("INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
                  "(1,2,600)");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
        ec = stmt("INSERT INTO recipe_step_localisation "
                  "(recipe_step_id,locale_id,description) VALUES "
                  "(1,3,'Food 2 step description')");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
    }
};

TEST_F(InsertRecipeStepTest, insert_when_basic_case)
{
    wholth::Context ctx{};
    ctx.locale_id = "3";

    wholth::entity::RecipeStep step{
        .time = "3h4m23s",
        .description = "1. cut;\n"
                       "2. dice;\n"
                       "3. chop;\n"
                       "4. fry;\n"
                       "5. stir.\n"};
    wholth::entity::Food food{.id = "1"};
    std::string result_id;
    auto ec = wholth::controller::insert(step, result_id, db_con, ctx.locale_id, food);

    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_TRUE(result_id.size() > 0);

    // 11063
    /* constexpr auto expected_seconds = 3 * 60 * 60 + 4 * 60 + 23; */
    std::stringstream ss;
    ec = sqlw::Statement{&db_con}(
        R"sql(
        SELECT
            rs.seconds,
            rsl.description
        FROM recipe_step rs
        LEFT JOIN recipe_step_localisation rsl
            ON rs.id = rsl.recipe_step_id
        WHERE rs.recipe_id = 1
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_STREQ2(
        "seconds:11063;description:"
        "1. cut;\n"
        "2. dice;\n"
        "3. chop;\n"
        "4. fry;\n"
        "5. stir.\n"
        ";",
        ss.str());

    ss = {};
    ec = sqlw::Statement{&db_con}(
        R"sql(
        SELECT
            rs.seconds,
            rsl.description
        FROM recipe_step rs
        LEFT JOIN recipe_step_localisation rsl
            ON rs.id = rsl.recipe_step_id
        WHERE rs.id = ?1
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; },
        std::array<sqlw::Statement::bindable_t, 1>{
            {{result_id, sqlw::Type::SQL_INT}}});
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_STREQ2(
        "seconds:11063;description:"
        "1. cut;\n"
        "2. dice;\n"
        "3. chop;\n"
        "4. fry;\n"
        "5. stir.\n"
        ";",
        ss.str());
}

TEST_F(InsertRecipeStepTest, insert_when_bad_locale_id)
{
    wholth::Context ctx{};
    ctx.locale_id = "3bad";

    wholth::entity::RecipeStep step{
        .time = "3h4m23s",
        .description = "1. cut;\n"
                       "2. dice;\n"
                       "3. chop;\n"
                       "4. fry;\n"
                       "5. stir.\n"};
    wholth::entity::Food food{.id = "1"};
    std::string result_id;
    auto ec = wholth::controller::insert(step, result_id, db_con, ctx.locale_id, food);

    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec << ec.message();
    ASSERT_TRUE(wholth::controller::insert_recipe_step_Code::OK != ec)
        << ec << ec.message();
    ASSERT_TRUE(wholth::status::Code::INVALID_LOCALE_ID == ec)
        << ec << ec.message();
    ASSERT_TRUE(result_id.size() == 0);

    std::stringstream ss;
    ec = sqlw::Statement{&db_con}(
        R"sql(
        SELECT
            rs.seconds,
            rsl.description
        FROM recipe_step rs
        LEFT JOIN recipe_step_localisation rsl
            ON rs.id = rsl.recipe_step_id
        WHERE rs.recipe_id = 1
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_STREQ2("", ss.str());
}

TEST_F(InsertRecipeStepTest, insert_when_another_step_diff_locale)
{
    wholth::Context ctx{};
    ctx.locale_id = "2";

    wholth::entity::RecipeStep step{
        .time = "3h4m23s", .description = "Second Step"};
    wholth::entity::Food food{.id = "2"};
    std::string result_id;
    auto ec = wholth::controller::insert(step, result_id, db_con, ctx.locale_id, food);

    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_TRUE(result_id.size() > 0);

    // 11063
    /* constexpr auto expected_seconds = 3 * 60 * 60 + 4 * 60 + 23; */
    std::stringstream ss;
    ec = sqlw::Statement{&db_con}(
        R"sql(
        SELECT
            rsl.locale_id,
            rs.seconds,
            rsl.description
        FROM recipe_step rs
        LEFT JOIN recipe_step_localisation rsl
            ON rs.id = rsl.recipe_step_id
        WHERE rs.recipe_id = 2
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_STREQ2(
        "locale_id:3;seconds:600;description:Food 2 step description;"
        "locale_id:2;seconds:11063;description:Second Step;",
        ss.str());
}

TEST_F(InsertRecipeStepTest, insert_when_invalid_recipe_time)
{
    wholth::Context ctx{};
    ctx.locale_id = "2";

    wholth::entity::RecipeStep step{
        .time = "3h4m23s 22years", .description = "Second Step"};
    wholth::entity::Food food{.id = "2"};
    std::string result_id;
    auto ec = wholth::controller::insert(step, result_id, db_con, ctx.locale_id, food);

    ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec << ec.message();
    ASSERT_TRUE(
        wholth::controller::insert_recipe_step_Code::INVALID_RECIPE_TIME == ec)
        << ec << ec.message();
    ASSERT_TRUE(result_id.size() == 0);

    std::stringstream ss;
    ec = sqlw::Statement{&db_con}(
        R"sql(
        SELECT
            rsl.locale_id,
            rs.seconds,
            rsl.description
        FROM recipe_step rs
        LEFT JOIN recipe_step_localisation rsl
            ON rs.id = rsl.recipe_step_id
        WHERE rs.recipe_id = 2
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec << ec.message();
    ASSERT_STREQ2(
        "locale_id:3;seconds:600;description:Food 2 step "
        "description;",
        ss.str());
}
