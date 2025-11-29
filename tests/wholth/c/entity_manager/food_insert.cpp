#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/app.h"
#include "wholth/c/entity_manager/food.h"
#include "wholth/c/entity/food.h"
#include "wholth/entity_manager/food.hpp"
#include <unistd.h>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_em_food_insert : public ApplicationAwareTest
{
};

TEST_F(Test_wholth_em_food_insert, when_good_case)
{
    auto& con = db::connection();
    wholth_user_locale_id(wtsv("2"));

    // wholth_Food food{
    //     .id = wtsv("22"),
    //     .title = wtsv(" Tomator   "),
    // };
    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("22");
    food.title = wtsv(" Tomator   ");
    food.description = wtsv("A red thing");
    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };
    //

    // auto err = wholth_em_insert_food(&food, &deets);
    auto buff = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_insert(&food, buff);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK == ec) << ec;
    ASSERT_STRNE2("22", wfsv(food.id));

    sqlw::Statement stmt{&db::connection()};
    std::string id{"bogus"};

    astmt(
        con,
        fmt::format("SELECT id FROM food WHERE id = {}", wfsv(food.id)),
        [&](auto e) { id = e.column_value; });
    ASSERT_STREQ3(wfsv(food.id), id);

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(
        con,
        fmt::format(
            "SELECT title, description "
            "FROM food_localisation "
            "WHERE food_id = {} AND locale_id = 2",
            wfsv(food.id)),
        [&](auto e) {
            if (e.column_name == "title")
            {
                title = e.column_value;
            }
            else
            {
                description = e.column_value;
            }
        });
    ASSERT_STREQ2("tomator", title) << "title should be trimmed and lowercased";
    ASSERT_STREQ2("A red thing", description);
}

TEST_F(Test_wholth_em_food_insert, when_duplicate)
{
    wholth_user_locale_id(wtsv("1"));

    auto& con = db::connection();

    std::string original_count;
    std::error_code ec;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        original_count = e.column_value;
    });
    ASSERT_TRUE(original_count.size() > 0)
        << "Couldn't get count of foods, so something's wrong!";

    std::string kale_count;
    astmt(
        con,
        "SELECT COUNT(id) FROM food f "
        "INNER JOIN food_localisation fl "
        "ON fl.food_id = f.id AND fl.locale_id = 1 "
        "WHERE fl.title = 'kale'",
        [&](auto e) { kale_count = e.column_value; });
    ASSERT_TRUE(kale_count == "1") << "Kale should be in database!";

    // wholth_Food food{
    //     .title = wtsv("Kale"),
    // };
    wholth_Food food = wholth_entity_food_init();
    // food.id = wtsv("22");
    food.title = wtsv(" Kale   ");
    food.description = wtsv("A red thing");
    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_insert_food(&food, &deets);
    auto buff = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_insert(&food, buff);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;

    std::string new_count;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_TRUE(new_count.size() > 0)
        << "Couldn't get new count of foods, so something's wrong!";
    ASSERT_TRUE(new_count == original_count)
        << "Count of foods changed, but food's a duplicate, you dummy!";
}

TEST_F(Test_wholth_em_food_insert, when_food_is_nullptr)
{
    wholth_user_locale_id(wtsv("1"));

    auto& con = db::connection();

    std::string original_count;
    std::error_code ec;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        original_count = e.column_value;
    });
    ASSERT_TRUE(original_count.size() > 0)
        << "Couldn't get count of foods, so something's wrong!";

    std::string kale_count;
    astmt(
        con,
        "SELECT COUNT(id) FROM food f "
        "INNER JOIN food_localisation fl "
        "ON fl.food_id = f.id AND fl.locale_id = 1 "
        "WHERE fl.title = 'kale'",
        [&](auto e) { kale_count = e.column_value; });
    ASSERT_TRUE(kale_count == "1") << "Kale should be in database!";

    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_insert_food(NULL, &deets);
    auto buff = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_insert(NULL, buff);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;
    ASSERT_TRUE(wholth::entity_manager::food::Code::FOOD_NULL == ec) << ec;

    std::string new_count;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_TRUE(new_count.size() > 0)
        << "Couldn't get new count of foods, so something's wrong!";
    ASSERT_TRUE(new_count == original_count)
        << "Count of foods changed, but food's a duplicate, you dummy!";
}

// TEST_F(Test_wholth_em_insert_food, when_deets_is_nullptr)
// {
//     wholth_app_locale_id(wtsv("1"));
//
//     auto& con = db::connection();
//
//     std::string original_count;
//     std::error_code ec;
//     astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
//         original_count = e.column_value;
//     });
//     ASSERT_TRUE(original_count.size() > 0)
//         << "Couldn't get count of foods, so something's wrong!";
//
//     std::string kale_count;
//     astmt(
//         con,
//         "SELECT COUNT(id) FROM food f "
//         "INNER JOIN food_localisation fl "
//         "ON fl.food_id = f.id AND fl.locale_id = 1 "
//         "WHERE fl.title = 'kale'",
//         [&](auto e) { kale_count = e.column_value; });
//     ASSERT_TRUE(kale_count == "1") << "Kale should be in database!";
//
//     wholth_Food food{
//         .title = wtsv("Kale"),
//     };
//
//     auto err = wholth_em_insert_food(&food, NULL);
//
//     ASSERT_NE(wholth_Error_OK.code, err.code) << err.code <<
//     wfsv(err.message); ASSERT_NE(wholth_Error_OK.message.size,
//     err.message.size)
//         << err.code << wfsv(err.message);
//     ec = wholth::entity_manager::food::Code(err.code);
//     ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;
//     ASSERT_TRUE(wholth::entity_manager::food::Code::NULL_DETAILS == ec) <<
//     ec;
//
//     std::string new_count;
//     astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
//         new_count = e.column_value;
//     });
//     ASSERT_TRUE(new_count.size() > 0)
//         << "Couldn't get new count of foods, so something's wrong!";
//     ASSERT_TRUE(new_count == original_count)
//         << "Count of foods changed, but food's a duplicate, you dummy!";
// }

TEST_F(Test_wholth_em_food_insert, when_title_is_null)
{
    wholth_user_locale_id(wtsv("2"));

    auto& con = db::connection();

    std::string original_count;
    std::error_code ec;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        original_count = e.column_value;
    });
    ASSERT_TRUE(original_count.size() > 0)
        << "Couldn't get count of foods, so something's wrong!";

    std::string kale_count;
    astmt(
        con,
        "SELECT COUNT(id) FROM food f "
        "INNER JOIN food_localisation fl "
        "ON fl.food_id = f.id AND fl.locale_id = 1 "
        "WHERE fl.title = 'kale'",
        [&](auto e) { kale_count = e.column_value; });
    ASSERT_TRUE(kale_count == "1") << "Kale should be in database!";

    // wholth_Food food{
    //     .title = wtsv("Kale"),
    // };
    wholth_Food food = wholth_entity_food_init();
    // food.title = wtsv("Kale");
    food.description = wtsv("A red thing");

    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_insert_food(&food, &deets);
    auto buff = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_insert(&food, buff);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;
    ASSERT_TRUE(wholth::entity_manager::food::Code::FOOD_NULL_TITLE == ec)
        << ec;

    std::string new_count;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_TRUE(new_count.size() > 0)
        << "Couldn't get new count of foods, so something's wrong!";
    ASSERT_TRUE(new_count == original_count)
        << "Count of foods changed, but food's a duplicate, you dummy!";
}

TEST_F(Test_wholth_em_food_insert, when_description_is_null)
{
    wholth_user_locale_id(wtsv("2"));

    auto& con = db::connection();

    std::string original_count;
    std::error_code ec;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        original_count = e.column_value;
    });
    ASSERT_TRUE(original_count.size() > 0)
        << "Couldn't get count of foods, so something's wrong!";

    std::string kale_count;
    astmt(
        con,
        "SELECT COUNT(id) FROM food f "
        "INNER JOIN food_localisation fl "
        "ON fl.food_id = f.id AND fl.locale_id = 1 "
        "WHERE fl.title = 'kale'",
        [&](auto e) { kale_count = e.column_value; });
    ASSERT_TRUE(kale_count == "1") << "Kale should be in database!";

    // wholth_Food food{
    //     .title = wtsv("Kale"),
    // };
    wholth_Food food = wholth_entity_food_init();
    food.title = wtsv("Kale");
    // food.description = wtsv("A red thing");

    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_insert_food(&food, &deets);
    auto buff = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_insert(&food, buff);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK == ec) << ec;
    // ASSERT_TRUE(wholth::entity_manager::food::Code::FOOD_NULL_DESCRIPTION ==
    // ec) << ec;

    std::string new_count;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_TRUE(new_count.size() > 0)
        << "Couldn't get new count of foods, so something's wrong!";
    ASSERT_TRUE(new_count != original_count);

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(
        con,
        fmt::format(
            "SELECT title, description "
            "FROM food_localisation "
            "WHERE food_id = {} AND locale_id = 2",
            wfsv(food.id)),
        [&](auto e) {
            if (e.column_name == "title")
            {
                title = e.column_value;
            }
            else
            {
                description = e.column_value;
            }
        });
    ASSERT_STREQ2("kale", title) << "title should be trimmed and lowercased";
    ASSERT_STREQ2("", description)
        << "description should not be set if not provided by user";
}

TEST_F(Test_wholth_em_food_insert, when_null_buffer)
{
    wholth_user_locale_id(wtsv("2"));

    auto& con = db::connection();

    std::string original_count;
    std::error_code ec;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        original_count = e.column_value;
    });
    ASSERT_TRUE(original_count.size() > 0)
        << "Couldn't get count of foods, so something's wrong!";

    std::string kale_count;
    astmt(
        con,
        "SELECT COUNT(id) FROM food f "
        "INNER JOIN food_localisation fl "
        "ON fl.food_id = f.id AND fl.locale_id = 1 "
        "WHERE fl.title = 'kale'",
        [&](auto e) { kale_count = e.column_value; });
    ASSERT_TRUE(kale_count == "1") << "Kale should be in database!";

    // wholth_Food food{
    //     .title = wtsv("Kale"),
    // };
    wholth_Food food = wholth_entity_food_init();
    food.title = wtsv("Kale");
    food.description = wtsv("A red thing");

    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_insert_food(&food, &deets);
    // auto buff = wholth_buffer_create();
    auto err = wholth_em_food_insert(&food, nullptr);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;

    std::string new_count;
    astmt(con, "SELECT COUNT(id) FROM food", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_TRUE(new_count.size() > 0)
        << "Couldn't get new count of foods, so something's wrong!";
    ASSERT_TRUE(new_count == original_count)
        << "Count of foods changed, but food's a duplicate, you dummy!";
}
