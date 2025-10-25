#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity/recipe_step.h"
#include "wholth/c/entity_manager/ingredient.h"
#include "wholth/c/forward.h"
#include "wholth/entity_manager/ingredient.hpp"
#include "wholth/entity_manager/recipe_step.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <sstream>
#include <type_traits>

static_assert(nullptr == (void*)NULL);

class Test_wholth_em_ingredient_insert : public ApplicationAwareTest
{
};

TEST_F(Test_wholth_em_ingredient_insert, when_basic_case)
{
    auto& con = db::connection();

    astmt(
        con,
        "INSERT INTO food (id, created_at) "
        "VALUES "
        " (199991,'10-10-2010'),"
        " (199992,'10-10-2010'),"
        " (199993,'10-10-2010')");
    astmt(
        con,
        "INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
        "(299991,199992,600)");
    astmt(
        con,
        "INSERT INTO recipe_step_food (recipe_step_id, food_id, "
        "canonical_mass) VALUES (299991,199991,100)");

    // const wholth_Ingredient ing{
    //     .food_id = wtsv("199993"),
    //     .canonical_mass_g = wtsv("300.12"),
    // };
    wholth_Ingredient ing = wholth_entity_ingredient_init();
    ing.food_id = wtsv("199993"),
    ing.canonical_mass_g = wtsv("300.12");
    // wholth_RecipeStep step{.id = wtsv("299991")};
    wholth_RecipeStep step = wholth_entity_recipe_step_init();
    step.id = wtsv("299991");

    auto buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_ingredient_insert(&ing, &step, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::ingredient::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::ingredient::Code::OK == ec)
        << ec << ec.message();

    ASSERT_GT(ing.id.size, 0);
    const auto result_id = wfsv(ing.id);
    ASSERT_TRUE(wholth::utils::is_valid_id(result_id)) << result_id;

    std::stringstream ss;
    astmt(
        con,
        R"sql(
        SELECT
            rsf.food_id,
            rsf.canonical_mass
        FROM recipe_step_food rsf
        WHERE rsf.recipe_step_id = 299991
        ORDER BY food_id ASC
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_STREQ2(
        "food_id:199991;canonical_mass:100;food_id:199993;canonical_mass:300."
        "12;",
        ss.str());
}

TEST_F(Test_wholth_em_ingredient_insert, when_duplicate)
{
    auto& con = db::connection();

    astmt(
        con,
        "INSERT INTO food (id, created_at) "
        "VALUES "
        " (199991,'10-10-2010'),"
        " (199992,'10-10-2010'),"
        " (199993,'10-10-2010')");
    astmt(
        con,
        "INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
        "(299991,199992,600)");
    astmt(
        con,
        "INSERT INTO recipe_step_food (recipe_step_id, food_id, "
        "canonical_mass) VALUES (299991,199991,100)");

    // const wholth_Ingredient ing{
    //     .food_id = wtsv("199991"),
    //     .canonical_mass_g = wtsv("300"),
    // };
    // wholth_RecipeStep step{.id = wtsv("299991")};
    //
    // const auto err = wholth_em_insert_ingredient(&ing, &step);
    //
    wholth_Ingredient ing = wholth_entity_ingredient_init();
    ing.food_id = wtsv("199991"),
    ing.canonical_mass_g = wtsv("300");
    wholth_RecipeStep step = wholth_entity_recipe_step_init();
    step.id = wtsv("299991");

    auto buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_ingredient_insert(&ing, &step, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::ingredient::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::ingredient::Code::OK != ec)
        << ec << ec.message();

    ASSERT_EQ(ing.id.size, 0);

    std::stringstream ss;
    astmt(
        con,
        R"sql(
        SELECT
            rsf.food_id,
            rsf.canonical_mass
        FROM recipe_step_food rsf
        WHERE rsf.recipe_step_id = 299991
        ORDER BY food_id ASC
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_STREQ2("food_id:199991;canonical_mass:100;", ss.str());
}

TEST_F(Test_wholth_em_ingredient_insert, when_nonexistent_step_id)
{
    auto& con = db::connection();

    astmt(
        con,
        "INSERT INTO food (id, created_at) "
        "VALUES "
        " (199991,'10-10-2010'),"
        " (199992,'10-10-2010'),"
        " (199993,'10-10-2010')");
    astmt(
        con,
        "INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
        "(299991,199992,600)");
    astmt(
        con,
        "INSERT INTO recipe_step_food (recipe_step_id, food_id, "
        "canonical_mass) VALUES (299991,199991,100)");

    // const wholth_Ingredient ing{
    //     .food_id = wtsv("199991"),
    //     .canonical_mass_g = wtsv("300"),
    // };
    // wholth_RecipeStep step{.id = wtsv("299997")};
    //
    // const auto err = wholth_em_insert_ingredient(&ing, &step);

    wholth_Ingredient ing = wholth_entity_ingredient_init();
    ing.food_id = wtsv("199991"),
    ing.canonical_mass_g = wtsv("300");
    wholth_RecipeStep step = wholth_entity_recipe_step_init();
    step.id = wtsv("299997");

    auto buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_ingredient_insert(&ing, &step, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::ingredient::Code(err.code);
    ASSERT_TRUE(
        wholth::entity_manager::ingredient::Code::INGREDIENT_POSTCONDITION_FAILED == ec)
        << ec << ec.message();

    std::stringstream ss;
    astmt(
        con,
        R"sql(
        SELECT
            rsf.food_id,
            rsf.canonical_mass
        FROM recipe_step_food rsf
        WHERE rsf.recipe_step_id = 299991
        ORDER BY food_id ASC
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_STREQ2("food_id:199991;canonical_mass:100;", ss.str());
}

TEST_F(Test_wholth_em_ingredient_insert, when_food_id_is_equal_to_recipe_id)
{
    auto& con = db::connection();

    astmt(
        con,
        "INSERT INTO food (id, created_at) "
        "VALUES "
        " (199991,'10-10-2010'),"
        " (199992,'10-10-2010'),"
        " (199993,'10-10-2010')");
    astmt(
        con,
        "INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
        "(299991,199992,600)");
    astmt(
        con,
        "INSERT INTO recipe_step_food (recipe_step_id, food_id, "
        "canonical_mass) VALUES (299991,199991,100)");

    // const wholth_Ingredient ing{
    //     .food_id = wtsv("199992"),
    //     .canonical_mass_g = wtsv("300"),
    // };
    // wholth_RecipeStep step{.id = wtsv("299991")};
    //
    // const auto err = wholth_em_insert_ingredient(&ing, &step);

    wholth_Ingredient ing = wholth_entity_ingredient_init();
    ing.food_id = wtsv("199992"),
    ing.canonical_mass_g = wtsv("300");
    wholth_RecipeStep step = wholth_entity_recipe_step_init();
    step.id = wtsv("299991");

    auto buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_ingredient_insert(&ing, &step, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::ingredient::Code(err.code);
    ASSERT_TRUE(
        wholth::entity_manager::ingredient::Code::INGREDIENT_POSTCONDITION_FAILED == ec)
        << ec << ec.message();

    std::stringstream ss;
    astmt(
        con,
        R"sql(
        SELECT
            rsf.food_id,
            rsf.canonical_mass
        FROM recipe_step_food rsf
        WHERE rsf.recipe_step_id = 299991
        ORDER BY food_id ASC
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_STREQ2("food_id:199991;canonical_mass:100;", ss.str());
}

TEST_F(Test_wholth_em_ingredient_insert, when_ingredient_is_null)
{
    auto& con = db::connection();

    std::string old_count;
    astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_GT(old_count.size(), 0);

    // const wholth_Ingredient ing{
    //     .food_id = wtsv("199992"),
    //     .canonical_mass_g = wtsv("300"),
    // };
    // wholth_RecipeStep step{.id = wtsv("299991")};
    //
    // const auto err = wholth_em_insert_ingredient(NULL, &step);

    wholth_RecipeStep step = wholth_entity_recipe_step_init();
    step.id = wtsv("299991");

    auto buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_ingredient_insert(NULL, &step, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::ingredient::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::ingredient::Code::INGREDIENT_IS_NULL == ec)
        << ec << ec.message();

    std::string new_count;
    astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_GT(new_count.size(), 0);
    ASSERT_STREQ3(old_count, new_count);
}

TEST_F(Test_wholth_em_ingredient_insert, when_recipe_step_is_null)
{
    auto& con = db::connection();

    std::string old_count;
    astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_GT(old_count.size(), 0);

    // const wholth_Ingredient ing{
    //     .food_id = wtsv("199992"),
    //     .canonical_mass_g = wtsv("300"),
    // };
    // // wholth_RecipeStep step{.id = wtsv("299991")};
    //
    // const auto err = wholth_em_insert_ingredient(&ing, NULL);

    wholth_Ingredient ing = wholth_entity_ingredient_init();
    ing.food_id = wtsv("199992"),
    ing.canonical_mass_g = wtsv("300");

    auto buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_ingredient_insert(&ing, NULL, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::recipe_step::Code(err.code);
    ASSERT_TRUE(
        wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL == ec)
        << ec << ec.message();

    std::string new_count;
    astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_GT(new_count.size(), 0);
    ASSERT_STREQ3(old_count, new_count);
}

TEST_F(Test_wholth_em_ingredient_insert, when_recipe_step_id_is_bogus)
{
    auto& con = db::connection();

    std::string old_count;
    astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_GT(old_count.size(), 0);

    // const wholth_Ingredient ing{
    //     .food_id = wtsv("199992"),
    //     .canonical_mass_g = wtsv("300"),
    // };
    wholth_Ingredient ing = wholth_entity_ingredient_init();
    ing.food_id = wtsv("199992");
    ing.canonical_mass_g = wtsv("300");

    std::vector<const wholth_RecipeStep> cases{{
        {},                   //
        {.id = wtsv("")},     //
        {.id = wtsv("abob")}, //
        {.id = wtsv("12-4")}, //
        // {.id={"12", 3}}, // Не уверен насчет валидности этого кейса.
    }};

    for (const auto& step : cases)
    {
        // const auto err = wholth_em_insert_ingredient(&ing, &step);
        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        const auto err = wholth_em_ingredient_insert(&ing, &step, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec = wholth::entity_manager::recipe_step::Code(err.code);
        ASSERT_TRUE(
            wholth::entity_manager::recipe_step::Code::RECIPE_STEP_INVALID_ID ==
            ec)
            << ec << ec.message();

        std::string new_count;
        astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_GT(new_count.size(), 0);
        ASSERT_STREQ3(old_count, new_count);
    }
}

TEST_F(Test_wholth_em_ingredient_insert, when_ingredient_id_is_bogus)
{
    auto& con = db::connection();

    std::string old_count;
    astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_GT(old_count.size(), 0);

    // wholth_RecipeStep step{.id = wtsv("299991")};
    wholth_RecipeStep step = wholth_entity_recipe_step_init();
    step.id = wtsv("299991");

    std::vector<wholth_Ingredient> cases{{
        {},
        {.food_id = wtsv("")},
        {.food_id = wtsv("abob")},
        {.food_id = wtsv("12-4")},
        // {.id={"12", 3}}, // Не уверен насчет валидности этого кейса.
    }};

    for (auto& ing : cases)
    {
        // const auto err = wholth_em_insert_ingredient(&ing, &step);
        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        const auto err = wholth_em_ingredient_insert(&ing, &step, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec = wholth::entity_manager::ingredient::Code(err.code);
        ASSERT_TRUE(
            wholth::entity_manager::ingredient::Code::INGREDIENT_INVALID_FOOD_ID ==
            ec)
            << ec << ec.message();

        std::string new_count;
        astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_GT(new_count.size(), 0);
        ASSERT_STREQ3(old_count, new_count);
    }
}

TEST_F(Test_wholth_em_ingredient_insert, when_ingredient_mass_is_bogus)
{
    auto& con = db::connection();

    std::string old_count;
    astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_GT(old_count.size(), 0);

    wholth_RecipeStep step{.id = wtsv("299991")};

    std::vector<wholth_Ingredient> cases{{
        {
            .food_id = wtsv("123"),
        },
        {.food_id = wtsv("123"), .canonical_mass_g = wtsv("sdid")},
        {.food_id = wtsv("123"), .canonical_mass_g = wtsv("123.04f")},
    }};

    for (auto& ing : cases)
    {
        // const auto err = wholth_em_insert_ingredient(&ing, &step);
        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        const auto err = wholth_em_ingredient_insert(&ing, &step, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec = wholth::entity_manager::ingredient::Code(err.code);
        ASSERT_TRUE(
            wholth::entity_manager::ingredient::Code::INGREDIENT_INVALID_MASS == ec)
            << ec << ec.message();

        std::string new_count;
        astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_GT(new_count.size(), 0);
        ASSERT_STREQ3(old_count, new_count);
    }
}

TEST_F(Test_wholth_em_ingredient_insert, when_buffer_is_nullptr)
{
    auto& con = db::connection();

    std::string old_count;
    astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_GT(old_count.size(), 0);

    auto ing = wholth_entity_ingredient_init();
    wholth_RecipeStep step{.id = wtsv("299991")};

    const auto err = wholth_em_ingredient_insert(&ing, &step, NULL);

    ASSERT_NE(wholth_Error_OK.code, err.code)
        << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::ingredient::Code(err.code);
    ASSERT_TRUE(
        wholth::entity_manager::ingredient::Code::OK != ec)
        << ec << ec.message();

    std::string new_count;
    astmt(con, "SELECT COUNT(*) FROM recipe_step_food", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_GT(new_count.size(), 0);
    ASSERT_STREQ3(old_count, new_count);
}
