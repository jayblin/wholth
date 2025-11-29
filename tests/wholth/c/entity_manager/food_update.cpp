#include "db/db.hpp"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/app.h"
#include "wholth/c/buffer.h"
#include "wholth/c/entity_manager/food.h"
#include "wholth/c/entity/food.h"
#include "wholth/entity_manager/food.hpp"

static_assert(nullptr == (void*)NULL);

class Test_wholth_em_food_update : public ApplicationAwareTest
{
};

using bind_t = sqlw::Statement::bindable_t;

TEST_F(Test_wholth_em_food_update, good_case)
{
    auto& con = db::connection();
    wholth_user_locale_id(wtsv("2"));

    std::string diff_locale_title{"bogus"};
    std::string diff_locale_description{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 1",
        [&](auto e) {
            if (e.column_name == "title")
            {
                diff_locale_title = e.column_value;
            }
            else
            {
                diff_locale_description = e.column_value;
            }
        });
    ASSERT_TRUE(diff_locale_title != "bogus") << "Test precondition failed!";
    ASSERT_TRUE(diff_locale_description != "bogus")
        << "Test precondition failed!";

    // wholth_Food food{
    //     .id = wtsv("100"),
    //     .title = wtsv(" Tomator   "),
    // };
    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("100");
    food.title = wtsv(" Tomator   ");
    food.description = wtsv("A red thing");
    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_update_food(&food, &deets);
    auto buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_update(&food, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK == ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
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
    ASSERT_STREQ2("tomator", title)
        << "title should be updated, trimmed and lowercased";
    ASSERT_STREQ2("A red thing", description)
        << "description shoould be updated";

    std::string diff_locale_title_new{"bogus"};
    std::string diff_locale_description_new{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 1",
        [&](auto e) {
            if (e.column_name == "title")
            {
                diff_locale_title_new = e.column_value;
            }
            else
            {
                diff_locale_description_new = e.column_value;
            }
        });
    ASSERT_STREQ3(diff_locale_title, diff_locale_title_new)
        << "Title of different lcoale should not be changed!";
    ASSERT_STREQ3(diff_locale_description, diff_locale_description_new)
        << "Description of different lcoale should not be changed!";
}

TEST_F(Test_wholth_em_food_update, when_food_is_nullptr)
{
    auto& con = db::connection();
    wholth_user_locale_id(wtsv("2"));

    std::string title_old{"bogus"};
    std::string description_old{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
        [&](auto e) {
            if (e.column_name == "title")
            {
                title_old = e.column_value;
            }
            else
            {
                description_old = e.column_value;
            }
        });
    ASSERT_TRUE(title_old != "bogus") << "Test precondition failed!";
    ASSERT_TRUE(description_old != "bogus") << "Test precondition failed!";

    // wholth_Food food{
    //     .id = wtsv("100"),
    //     .title = wtsv(" Tomator   "),
    // };
    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_update_food(NULL, &deets);
    auto buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_update(NULL, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;
    ASSERT_TRUE(wholth::entity_manager::food::Code::FOOD_NULL == ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
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
    ASSERT_STREQ3(title_old, title) << "title should not be updated";
    ASSERT_STREQ3(description_old, description)
        << "description shoould not be updated";
}

TEST_F(Test_wholth_em_food_update, when_buffer_is_nullptr)
{
    auto& con = db::connection();
    wholth_user_locale_id(wtsv("2"));

    std::string title_old{"bogus"};
    std::string description_old{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
        [&](auto e) {
            if (e.column_name == "title")
            {
                title_old = e.column_value;
            }
            else
            {
                description_old = e.column_value;
            }
        });
    ASSERT_TRUE(title_old != "bogus") << "Test precondition failed!";
    ASSERT_TRUE(description_old != "bogus") << "Test precondition failed!";

    // wholth_Food food{
    //     .id = wtsv("100"),
    //     .title = wtsv(" Tomator   "),
    // };
    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_update_food(NULL, &deets);
    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("100");
    food.title = wtsv(" Tomator   ");
    food.description = wtsv("A red thing");

    auto err = wholth_em_food_update(&food, NULL);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
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
    ASSERT_STREQ3(title_old, title) << "title should not be updated";
    ASSERT_STREQ3(description_old, description)
        << "description shoould not be updated";
}

// TEST_F(Test_wholth_em_update_food, when_deets_is_nullptr)
// {
//     auto& con = db::connection();
//     wholth_app_locale_id(wtsv("2"));
//
//     std::string title_old{"bogus"};
//     std::string description_old{"bogus"};
//     astmt(
//         con,
//         "SELECT title, description "
//         "FROM food_localisation "
//         "WHERE food_id = 100 AND locale_id = 2",
//         [&](auto e) {
//             if (e.column_name == "title")
//             {
//                 title_old = e.column_value;
//             }
//             else
//             {
//                 description_old = e.column_value;
//             }
//         });
//     ASSERT_TRUE(title_old != "bogus") << "Test precondition failed!";
//     ASSERT_TRUE(description_old != "bogus") << "Test precondition failed!";
//
//     wholth_Food food{
//         .id = wtsv("100"),
//         .title = wtsv(" Tomator   "),
//     };
//     // wholth_FoodDetails deets{
//     //     .description = wtsv("A red thing"),
//     // };
//
//     auto err = wholth_em_update_food(&food, NULL);
//
//     ASSERT_NE(wholth_Error_OK.code, err.code) << err.code <<
//     wfsv(err.message); ASSERT_NE(wholth_Error_OK.message.size,
//     err.message.size)
//         << err.code << wfsv(err.message);
//     std::error_code ec = wholth::entity_manager::food::Code(err.code);
//     ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;
//     ASSERT_TRUE(wholth::entity_manager::food::Code::NULL_DETAILS == ec) <<
//     ec;
//
//     std::string title{"bogus"};
//     std::string description{"bogus"};
//     astmt(
//         con,
//         "SELECT title, description "
//         "FROM food_localisation "
//         "WHERE food_id = 100 AND locale_id = 2",
//         [&](auto e) {
//             if (e.column_name == "title")
//             {
//                 title = e.column_value;
//             }
//             else
//             {
//                 description = e.column_value;
//             }
//         });
//     ASSERT_STREQ3(title_old, title) << "title should not be updated";
//     ASSERT_STREQ3(description_old, description)
//         << "description shoould not be updated";
// }

TEST_F(Test_wholth_em_food_update, when_bad_food_id)
{
    auto& con = db::connection();
    wholth_user_locale_id(wtsv("2"));

    std::string diff_locale_title{"bogus"};
    std::string diff_locale_description{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 1",
        [&](auto e) {
            if (e.column_name == "title")
            {
                diff_locale_title = e.column_value;
            }
            else
            {
                diff_locale_description = e.column_value;
            }
        });
    ASSERT_TRUE(diff_locale_title != "bogus") << "Test precondition failed!";
    ASSERT_TRUE(diff_locale_description != "bogus")
        << "Test precondition failed!";

    std::string old_title{"bogus"};
    std::string old_description{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
        [&](auto e) {
            if (e.column_name == "title")
            {
                old_title = e.column_value;
            }
            else
            {
                old_description = e.column_value;
            }
        });
    ASSERT_TRUE(old_title != "bogus") << "Test precondition failed!";
    ASSERT_TRUE(old_description != "bogus") << "Test precondition failed!";

    std::vector<wholth_Food> cases{{
        {},
        {
            .id = wtsv(""),
        },
        {
            .id = wtsv("-33"),
        },
        {
            .id = wtsv("33s3"),
        },
    }};

    for (auto& food : cases)
    {
        // wholth_FoodDetails deets{
        //     .description = wtsv("A red thing"),
        // };

        // auto err = wholth_em_update_food(&food, &deets);
        auto buf = wholth_buffer_ring_pool_element();
        auto err = wholth_em_food_update(&food, buf);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);
        std::error_code ec = wholth::entity_manager::food::Code(err.code);
        ASSERT_NE(wholth::entity_manager::food::Code::OK, ec) << ec;
        ASSERT_EQ(wholth::entity_manager::food::Code::FOOD_INVALID_ID, ec)
            << ec;

        std::string title{"bogus"};
        std::string description{"bogus"};
        astmt(
            con,
            "SELECT title, description "
            "FROM food_localisation "
            "WHERE food_id = 100 AND locale_id = 2",
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
        ASSERT_STREQ3(old_title, title) << "title should not be updated";
        ASSERT_STREQ3(old_description, description)
            << "description should not be updated";

        std::string diff_locale_title_new{"bogus"};
        std::string diff_locale_description_new{"bogus"};
        astmt(
            con,
            "SELECT title, description "
            "FROM food_localisation "
            "WHERE food_id = 100 AND locale_id = 1",
            [&](auto e) {
                if (e.column_name == "title")
                {
                    diff_locale_title_new = e.column_value;
                }
                else
                {
                    diff_locale_description_new = e.column_value;
                }
            });
        ASSERT_STREQ3(diff_locale_title, diff_locale_title_new)
            << "Title of different lcoale should not be changed!";
        ASSERT_STREQ3(diff_locale_description, diff_locale_description_new)
            << "Description of different lcoale should not be changed!";
    }
}

TEST_F(Test_wholth_em_food_update, when_title_is_nullptr)
{
    auto& con = db::connection();
    wholth_user_locale_id(wtsv("2"));

    std::string title_old{"bogus"};
    std::string description_old{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
        [&](auto e) {
            if (e.column_name == "title")
            {
                title_old = e.column_value;
            }
            else
            {
                description_old = e.column_value;
            }
        });
    ASSERT_TRUE(title_old != "bogus") << "Test precondition failed!";
    ASSERT_TRUE(description_old != "bogus") << "Test precondition failed!";

    // wholth_Food food{
    //     .id = wtsv("100"),
    //     .title = wtsv(" Tomator   "),
    // };
    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_update_food(NULL, &deets);
    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("100");
    // food.title = wtsv(" Tomator   ");
    food.description = wtsv("A red thing");

    auto buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_update(&food, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK == ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
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
    ASSERT_STREQ3(title_old, title) << "title should not be updated";
    ASSERT_STREQ2("A red thing", description)
        << "description shoould be updated";
}

TEST_F(Test_wholth_em_food_update, when_description_is_nullptr)
{
    auto& con = db::connection();
    wholth_user_locale_id(wtsv("2"));

    std::string title_old{"bogus"};
    std::string description_old{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
        [&](auto e) {
            if (e.column_name == "title")
            {
                title_old = e.column_value;
            }
            else
            {
                description_old = e.column_value;
            }
        });
    ASSERT_TRUE(title_old != "bogus") << "Test precondition failed!";
    ASSERT_TRUE(description_old != "bogus") << "Test precondition failed!";

    // wholth_Food food{
    //     .id = wtsv("100"),
    //     .title = wtsv(" Tomator   "),
    // };
    // wholth_FoodDetails deets{
    //     .description = wtsv("A red thing"),
    // };

    // auto err = wholth_em_update_food(NULL, &deets);
    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("100");
    food.title = wtsv(" Tomator   ");
    // food.description = wtsv("A red thing");

    auto buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_update(&food, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK == ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(
        con,
        "SELECT title, description "
        "FROM food_localisation "
        "WHERE food_id = 100 AND locale_id = 2",
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
    ASSERT_STREQ2("tomator", title) << "title should be updated";
    ASSERT_STREQ3(description_old, description)
        << "description shoould not be updated";
}
