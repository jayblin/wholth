#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/entity_manager/food_nutrient.h"
#include "wholth/entity_manager/food.hpp"
#include "wholth/entity_manager/nutrient.hpp"
#include <unistd.h>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_em_food_nutrient_upsert : public ApplicationAwareTest
{
};

TEST_F(Test_wholth_em_food_nutrient_upsert, when_new_nutrient)
{
    auto& con = db::connection();

    astmt(con, "INSERT INTO food VALUES (999999,'2010-01-01')");
    astmt(
        con,
        "INSERT INTO food_nutrient "
        "VALUES (999999,1009,343)");

    // wholth_Food food{
    //     .id = wtsv("999999"),
    // };
    // wholth_Nutrient nutr{
    //     .id = wtsv("1008"),
    //     .value = wtsv("1234.5"),
    // };
    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("999999");
    wholth_Nutrient nutr = wholth_entity_nutrient_init();
    nutr.id = wtsv("1008");
    nutr.value = wtsv("1234.5");

    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_nutrient_upsert(&food, &nutr, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::stringstream ss;
    astmt(
        con,
        "SELECT food_id, nutrient_id, value FROM food_nutrient "
        "WHERE food_id = 999999 ORDER BY nutrient_id DESC",
        [&](auto e) { ss << e.column_value << ";"; });
    ASSERT_STREQ2("999999;1009;343;999999;1008;1234.5;", ss.str());
}

TEST_F(Test_wholth_em_food_nutrient_upsert, when_existing_nutrient)
{
    auto& con = db::connection();

    astmt(con, "INSERT INTO food VALUES (999999,'2010-01-01')");
    astmt(
        con,
        "INSERT INTO food_nutrient "
        "VALUES (999999,1009,343)");

    // wholth_Food food{
    //     .id = wtsv("999999"),
    // };
    // wholth_Nutrient nutr{
    //     .id = wtsv("1009"),
    //     .value = wtsv("117"),
    // };
    //
    // auto err = wholth_em_upsert_food_nutrient(&food, &nutr);

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("999999");
    wholth_Nutrient nutr = wholth_entity_nutrient_init();
    nutr.id = wtsv("1009");
    nutr.value = wtsv("117");

    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_nutrient_upsert(&food, &nutr, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::stringstream ss;
    astmt(
        con,
        "SELECT food_id, nutrient_id, value FROM food_nutrient "
        "WHERE food_id = 999999",
        [&](auto e) { ss << e.column_value << ";"; });
    ASSERT_STREQ2("999999;1009;117;", ss.str());
}

TEST_F(Test_wholth_em_food_nutrient_upsert, when_food_is_null)
{
    auto& con = db::connection();

    astmt(con, "INSERT INTO food VALUES (999999,'2010-01-01')");
    astmt(
        con,
        "INSERT INTO food_nutrient "
        "VALUES (999999,1009,343)");

    // // wholth_Food food{
    // //     .id = wtsv("999999"),
    // // };
    // wholth_Nutrient nutr{
    //     .id = wtsv("1009"),
    //     .value = wtsv("117"),
    // };
    //
    // auto err = wholth_em_upsert_food_nutrient(NULL, &nutr);

    wholth_Nutrient nutr = wholth_entity_nutrient_init();
    nutr.id = wtsv("1008");
    nutr.value = wtsv("1234.5");

    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_nutrient_upsert(NULL, &nutr, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::FOOD_NULL, ec) << ec;

    std::stringstream ss;
    astmt(
        con,
        "SELECT food_id, nutrient_id, value FROM food_nutrient "
        "WHERE food_id = 999999",
        [&](auto e) { ss << e.column_value << ";"; });
    ASSERT_STREQ2("999999;1009;343;", ss.str());
}

TEST_F(Test_wholth_em_food_nutrient_upsert, when_nutrient_is_null)
{
    auto& con = db::connection();

    astmt(con, "INSERT INTO food VALUES (999999,'2010-01-01')");
    astmt(
        con,
        "INSERT INTO food_nutrient "
        "VALUES (999999,1009,343)");

    // wholth_Food food{
    //     .id = wtsv("999999"),
    // };
    // // wholth_Nutrient nutr{
    // //     .id = wtsv("1009"),
    // //     .value = wtsv("117"),
    // // };
    //
    // auto err = wholth_em_upsert_food_nutrient(&food, NULL);

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("999999");

    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_nutrient_upsert(&food, NULL, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::nutrient::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::nutrient::Code::NUTRIENT_NULL, ec) << ec;

    std::stringstream ss;
    astmt(
        con,
        "SELECT food_id, nutrient_id, value FROM food_nutrient "
        "WHERE food_id = 999999",
        [&](auto e) { ss << e.column_value << ";"; });
    ASSERT_STREQ2("999999;1009;343;", ss.str());
}

TEST_F(Test_wholth_em_food_nutrient_upsert, when_nutrient_id_is_bogus)
{
    auto& con = db::connection();

    astmt(con, "INSERT INTO food VALUES (999999,'2010-01-01')");
    astmt(
        con,
        "INSERT INTO food_nutrient "
        "VALUES (999999,1009,343)");

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("999999");

    const std::vector<wholth_Nutrient> cases{{
        {},
        {.id = wtsv("")},
        {.id = wtsv("-40")},
        {.id = wtsv("40.3")},
        {.id = wtsv("1f3f4i")},
    }};

    auto i = 0;
    for (const auto& nutr : cases)
    {
        std::string idx = fmt::format("#{}. ", i);

        wholth_Buffer* buf = wholth_buffer_ring_pool_element();
        auto err = wholth_em_food_nutrient_upsert(&food, &nutr, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << idx << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << idx << err.code << wfsv(err.message);
        std::error_code ec = wholth::entity_manager::nutrient::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::nutrient::Code::NUTRIENT_INVALID_ID, ec)
            << idx << ec;

        std::stringstream ss;
        astmt(
            con,
            "SELECT food_id, nutrient_id, value FROM food_nutrient "
            "WHERE food_id = 999999",
            [&](auto e) { ss << e.column_value << ";"; });
        ASSERT_STREQ2("999999;1009;343;", ss.str()) << idx;
        i++;
    }
}

TEST_F(Test_wholth_em_food_nutrient_upsert, when_food_id_is_bogus)
{
    auto& con = db::connection();

    astmt(con, "INSERT INTO food VALUES (999999,'2010-01-01')");
    astmt(
        con,
        "INSERT INTO food_nutrient "
        "VALUES (999999,1009,343)");

    wholth_Nutrient nutr = wholth_entity_nutrient_init();
    nutr.id = wtsv("1009");
    nutr.value = wtsv("117");

    const std::vector<wholth_Food> cases{{
        {},
        {.id = wtsv("")},
        {.id = wtsv("-40")},
        {.id = wtsv("40.3")},
        {.id = wtsv("1f3f4i")},
    }};

    auto i = 0;
    for (const auto& food : cases)
    {
        std::string idx = fmt::format("#{}. ", i);

        wholth_Buffer* buf = wholth_buffer_ring_pool_element();

        auto err = wholth_em_food_nutrient_upsert(&food, &nutr, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << idx << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << idx << err.code << wfsv(err.message);
        std::error_code ec = wholth::entity_manager::food::Code(err.code);
        ASSERT_EQ(wholth::entity_manager::food::Code::FOOD_INVALID_ID, ec)
            << idx << ec;

        std::stringstream ss;
        astmt(
            con,
            "SELECT food_id, nutrient_id, value FROM food_nutrient "
            "WHERE food_id = 999999",
            [&](auto e) { ss << e.column_value << ";"; });
        ASSERT_STREQ2("999999;1009;343;", ss.str()) << idx;
        i++;
    }
}
TEST_F(Test_wholth_em_food_nutrient_upsert, when_buffer_is_nullptr)
{
    auto& con = db::connection();

    astmt(con, "INSERT INTO food VALUES (999999,'2010-01-01')");
    astmt(
        con,
        "INSERT INTO food_nutrient "
        "VALUES (999999,1009,343)");

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("999999");
    wholth_Nutrient nutr = wholth_entity_nutrient_init();
    nutr.id = wtsv("1008");
    nutr.value = wtsv("1234.5");

    auto err = wholth_em_food_nutrient_upsert(&food, &nutr, NULL);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::nutrient::Code(err.code);
    ASSERT_NE(wholth::entity_manager::nutrient::Code::OK, ec) << ec;

    std::stringstream ss;
    astmt(
        con,
        "SELECT food_id, nutrient_id, value FROM food_nutrient "
        "WHERE food_id = 999999",
        [&](auto e) { ss << e.column_value << ";"; });
    ASSERT_STREQ2("999999;1009;343;", ss.str());
}
