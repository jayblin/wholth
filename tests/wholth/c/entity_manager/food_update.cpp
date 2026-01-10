#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/connection.hpp"
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

constexpr std::string_view sql_check =
    "SELECT fl_fts5.title, fl_fts5.description "
    "FROM food_localisation_fts5 fl_fts5 "
    "INNER JOIN food_localisation fl "
    " ON fl.fl_fts5_rowid = fl_fts5.rowid "
    "WHERE fl.food_id = 100 AND fl.locale_id = {0}";

static std::pair<std::string, std::string> count_fl_and_fl_fts5(
    sqlw::Connection& con)
{
    std::string count1 = "";
    astmt(con, "SELECT COUNT(*) FROM food_localisation", [&](auto e) {
        count1 = e.column_value;
    });

    std::string count2 = "";
    astmt(con, "SELECT COUNT(rowid) FROM food_localisation_fts5", [&](auto e) {
        count2 = e.column_value;
    });

    return {count1, count2};
}

TEST_F(Test_wholth_em_food_update, when_food_has_localisation_in_both_locales)
{
    auto& con = db::connection();

    const auto counts_old = count_fl_and_fl_fts5(con);

    std::string diff_locale_title{"bogus"};
    std::string diff_locale_description{"bogus"};
    astmt(con, fmt::format(sql_check, "1"), [&](auto e) {
        if (e.column_name == "title")
        {
            diff_locale_title = e.column_value;
        }
        else
        {
            diff_locale_description = e.column_value;
        }
    });
    ASSERT_STRNEQ2("bogus", diff_locale_title) << "Test precondition failed!";
    ASSERT_STRNEQ2("bogus", diff_locale_description)
        << "Test precondition failed!";

    std::string cur_locale_title{"bogus"};
    std::string cur_locale_description{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
        if (e.column_name == "title")
        {
            cur_locale_title = e.column_value;
        }
        else
        {
            cur_locale_description = e.column_value;
        }
    });
    ASSERT_STRNEQ2("bogus", cur_locale_title) << "Test precondition failed!";
    ASSERT_STRNEQ2("bogus", cur_locale_description)
        << "Test precondition failed!";

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("100");
    food.title = wtsv(" Tomator   ");
    food.description = wtsv("A red thing");

    auto buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_update(&food, wtsv("2"), buf);

    ASSERT_WHOLTH_OK(err);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK == ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
        if (e.column_name == "title")
        {
            title = e.column_value;
        }
        else
        {
            description = e.column_value;
        }
    });
    ASSERT_STREQ2("Tomator", title) << "title should be updated and trimmed";
    ASSERT_STREQ2("A red thing", description)
        << "description shoould be updated";

    std::string diff_locale_title_new{"bogus"};
    std::string diff_locale_description_new{"bogus"};
    astmt(con, fmt::format(sql_check, "1"), [&](auto e) {
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

    const auto counts_new = count_fl_and_fl_fts5(con);
    ASSERT_STREQ3(counts_new.first, counts_old.first);
    ASSERT_STREQ3(counts_new.second, counts_old.second);
}

TEST_F(
    Test_wholth_em_food_update,
    when_food_has_localisation_in_only_one_locale)
{
    auto& con = db::connection();

    std::string diff_locale_title{"bogus"};
    std::string diff_locale_description{"bogus"};
    astmt(con, fmt::format(sql_check, "1"), [&](auto e) {
        if (e.column_name == "title")
        {
            diff_locale_title = e.column_value;
        }
        else
        {
            diff_locale_description = e.column_value;
        }
    });
    ASSERT_STRNEQ2("bogus", diff_locale_title) << "Test precondition failed!";
    ASSERT_STRNEQ2("bogus", diff_locale_description)
        << "Test precondition failed!";

    astmt(
        con,
        "DELETE FROM food_localisation_fts5 WHERE rowid IN "
        "(SELECT fl_fts5_rowid FROM food_localisation  "
        " WHERE food_id = 100 AND locale_id = 2)");
    astmt(
        con,
        "DELETE FROM food_localisation WHERE food_id = 100 AND locale_id = 2");
    std::string cur_locale_title{"bogus"};
    std::string cur_locale_description{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
        if (e.column_name == "title")
        {
            cur_locale_title = e.column_value;
        }
        else
        {
            cur_locale_description = e.column_value;
        }
    });
    ASSERT_STREQ2("bogus", cur_locale_title) << "Test precondition failed!";
    ASSERT_STREQ2("bogus", cur_locale_description)
        << "Test precondition failed!";

    const auto counts_old = count_fl_and_fl_fts5(con);

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("100");
    food.title = wtsv(" Tomator   ");
    food.description = wtsv("A red thing");

    auto buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_update(&food, wtsv("2"), buf);

    ASSERT_WHOLTH_OK(err);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK == ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
        if (e.column_name == "title")
        {
            title = e.column_value;
        }
        else
        {
            description = e.column_value;
        }
    });
    ASSERT_STREQ2("Tomator", title) << "title should be updated and trimmed";
    ASSERT_STREQ2("A red thing", description)
        << "description shoould be updated";

    std::string diff_locale_title_new{"bogus"};
    std::string diff_locale_description_new{"bogus"};
    astmt(con, fmt::format(sql_check, "1"), [&](auto e) {
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

    const auto counts_new = count_fl_and_fl_fts5(con);
    ASSERT_STRNE3(counts_new.first, counts_old.first);
    ASSERT_STRNE3(counts_new.second, counts_old.second);
}

TEST_F(Test_wholth_em_food_update, when_food_is_nullptr)
{
    auto& con = db::connection();
    std::string title_old{"bogus"};
    std::string description_old{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
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

    auto buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_update(NULL, wtsv("1"), buf);

    ASSERT_WHOLTH_NOK(err);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;
    ASSERT_TRUE(wholth::entity_manager::food::Code::FOOD_NULL == ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
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
    std::string title_old{"bogus"};
    std::string description_old{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
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

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("100");
    food.title = wtsv(" Tomator   ");
    food.description = wtsv("A red thing");

    auto err = wholth_em_food_update(&food, wtsv("1"), NULL);

    ASSERT_WHOLTH_NOK(err);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK != ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
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

TEST_F(Test_wholth_em_food_update, when_bad_food_id)
{
    auto& con = db::connection();
    std::string diff_locale_title{"bogus"};
    std::string diff_locale_description{"bogus"};
    astmt(con, fmt::format(sql_check, "1"), [&](auto e) {
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
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
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
        auto buf = wholth_buffer_ring_pool_element();
        auto err = wholth_em_food_update(&food, wtsv("1"), buf);

        ASSERT_WHOLTH_NOK(err);
        std::error_code ec = wholth::entity_manager::food::Code(err.code);
        ASSERT_NE(wholth::entity_manager::food::Code::OK, ec) << ec;
        ASSERT_EQ(wholth::entity_manager::food::Code::FOOD_INVALID_ID, ec)
            << ec;

        std::string title{"bogus"};
        std::string description{"bogus"};
        astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
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
        astmt(con, fmt::format(sql_check, "1"), [&](auto e) {
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
    std::string title_old{"bogus"};
    std::string description_old{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
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

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("100");
    food.description = wtsv("A red thing");

    auto buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_update(&food, wtsv("2"), buf);

    ASSERT_WHOLTH_OK(err);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK == ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
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
    std::string title_old{"bogus"};
    std::string description_old{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
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

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("100");
    food.title = wtsv(" Tomator   ");

    auto buf = wholth_buffer_ring_pool_element();
    auto err = wholth_em_food_update(&food, wtsv("2"), buf);

    ASSERT_WHOLTH_OK(err);
    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_TRUE(wholth::entity_manager::food::Code::OK == ec) << ec;

    std::string title{"bogus"};
    std::string description{"bogus"};
    astmt(con, fmt::format(sql_check, "2"), [&](auto e) {
        if (e.column_name == "title")
        {
            title = e.column_value;
        }
        else
        {
            description = e.column_value;
        }
    });
    ASSERT_STREQ2("Tomator", title) << "title should be updated";
    ASSERT_STREQ3(description_old, description)
        << "description shoould not be updated";
}
