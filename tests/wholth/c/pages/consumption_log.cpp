#include "gtest/gtest.h"
#include <type_traits>
#include "fmt/core.h"
#include "helpers.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity_manager/user.h"
#include "wholth/c/pages/utils.h"
#include "wholth/entity_manager/consumption_log.hpp"
#include "wholth/pages/code.hpp"
#include "wholth/c/pages/consumption_log.h"

static_assert(nullptr == (void*)NULL);

class Test_wholth_pages_consumption_log : public ApplicationAwareTest
{
  protected:
    static void SetUpTestSuite()
    {
        ApplicationAwareTest::SetUpTestSuite();
        astmt(db::connection(), "SAVEPOINT Test_wholth_pages_consumption_log");
        astmt(db::connection(), "DELETE FROM food_localisation WHERE 1=1");
        astmt(db::connection(), "DELETE FROM food_localisation_fts5 WHERE 1=1");
        astmt(db::connection(), "DELETE FROM food_nutrient WHERE 1=1");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO user "
            " (id, name, password_hashed, locale_id) VALUES "
            " (1, 'test', 'standalone', 1),"
            " (2, 'test-2', 'standalone', 1);");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO consumption_log "
            " (id, user_id, food_id, mass, consumed_at) VALUES "
            " (99990, 1, 1, 101, '2020-01-30T11:00:00'),"
            " (99991, 1, 2, 102, '2020-01-30T11:00:01'),"
            " (99992, 1, 3, 103, '2025-08-30T01:11:11'),"
            " (99993, 1, 4, 104, '2025-08-30T05:11:11'),"
            " (99994, 1, 5, 105, '2025-08-30T11:11:11'),"
            " (99995, 1, 6, 106, '2025-08-30T13:11:11'),"
            " (99996, 1, 7, 107, '2025-08-30T18:11:11'),"
            " (99998, 2, 7, 107, '2025-08-30T18:11:11'),"
            " (99999, 2, 7, 107, '2025-08-30T18:11:11'),"
            " (99997, 1, 8, 108, '2300-09-23T00:00:00');");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO food_localisation_fts5 "
            " (rowid, title) VALUES"
            " (1, 'cl-1-1'),"
            " (2, 'cl-1-2'),"
            " (3, 'cl-2-1'),"
            " (4, 'cl-3-1'),"
            " (5, 'cl-3-2'),"
            " (6, 'cl-4-1'),"
            " (7, 'cl-4-2'),"
            " (8, 'cl-5-1'),"
            " (9, 'cl-5-2'),"
            " (10, 'cl-6-1'),"
            " (11, 'cl-6-2'),"
            " (12, 'cl-7-1'),"
            " (13, 'cl-7-2'),"
            " (14, 'cl-8-1');");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO food_localisation "
            " (food_id, locale_id, fl_fts5_rowid) VALUES "
            " (1, 1, 1),"
            " (1, 2, 2),"
            " (2, 1, 3),"
            " (3, 1, 4),"
            " (3, 2, 5),"
            " (4, 1, 6),"
            " (4, 2, 7),"
            " (5, 1, 8),"
            " (5, 2, 9),"
            " (6, 1, 10),"
            " (6, 2, 11),"
            " (7, 1, 12),"
            " (7, 2, 13),"
            " (8, 1, 14);");
        astmt(
            db::connection(),
            "DELETE FROM food_localisation WHERE food_id IN (2,8) AND "
            "locale_id = 2");

        std::string top_nutrient_id;
        astmt(
            db::connection(),
            "SELECT id FROM nutrient WHERE position = 10",
            [&top_nutrient_id](sqlw::Statement::ExecArgs e) {
                top_nutrient_id = e.column_value;
            });

        ASSERT_NE(0, top_nutrient_id.size());
        astmt(
            db::connection(),
            fmt::format(
                "INSERT OR REPLACE INTO food_nutrient "
                "(food_id,nutrient_id,value) "
                "VALUES "
                "(1,  {0}, 120),"
                "(1, 1007, 117),"
                "(2,  {0}, 77),"
                "(2, 1098, 900)",
                top_nutrient_id));
    }

    static void TearDownTestSuite()
    {
        astmt(
            db::connection(), "ROLLBACK TO Test_wholth_pages_consumption_log");
    }
};

// should br executed first in this group
TEST_F(Test_wholth_pages_consumption_log, when_bad_dates)
{
    auto buf = wholth_buffer_ring_pool_element();
    wholth_em_user_locale_id(wtsv("1"), wtsv("1"), buf);

    // At this point query object should be empty
    {
        wholth_Page* page = nullptr;
        auto err = wholth_pages_consumption_log(&page, 8);
        auto wrap = PageWrap{page};
        ASSERT_WHOLTH_OK(err);
        err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_WHOLTH_OK(err);

        err = wholth_pages_fetch(page);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec =
            wholth::entity_manager::consumption_log::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::consumption_log::Code::
                CONSUMPTION_LOG_INVALID_DATE,
            ec)
            << ec << ec.message();

        const wholth_ConsumptionLogArray logs =
            wholth_pages_consumption_log_array(page);

        ASSERT_EQ(0, logs.size);
        ASSERT_EQ(nullptr, logs.data);
    }

    {
        wholth_Page* page = nullptr;
        auto err = wholth_pages_consumption_log(&page, 8);
        auto wrap = PageWrap{page};
        ASSERT_WHOLTH_OK(err);
        err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_WHOLTH_OK(err);

        err =
            wholth_pages_consumption_log_period(page, wtsv("from"), wtsv("to"));

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec =
            wholth::entity_manager::consumption_log::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::consumption_log::Code::
                CONSUMPTION_LOG_INVALID_DATE,
            ec)
            << ec << ec.message();

        err = wholth_pages_fetch(page);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        ec = wholth::entity_manager::consumption_log::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::consumption_log::Code::
                CONSUMPTION_LOG_INVALID_DATE,
            ec)
            << ec << ec.message();

        const wholth_ConsumptionLogArray logs =
            wholth_pages_consumption_log_array(page);

        ASSERT_EQ(0, logs.size);
        ASSERT_EQ(nullptr, logs.data);
    }

    {
        wholth_Page* page = nullptr;
        auto err = wholth_pages_consumption_log(&page, 8);
        auto wrap = PageWrap{page};
        ASSERT_WHOLTH_OK(err);

        err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_WHOLTH_OK(err);

        err = wholth_pages_consumption_log_period(
            page, wtsv("2022-02-02T22:22:22"), wtsv("to"));

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec =
            wholth::entity_manager::consumption_log::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::consumption_log::Code::
                CONSUMPTION_LOG_INVALID_DATE,
            ec)
            << ec << ec.message();

        err = wholth_pages_fetch(page);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        ec = wholth::entity_manager::consumption_log::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::consumption_log::Code::
                CONSUMPTION_LOG_INVALID_DATE,
            ec)
            << ec << ec.message();

        const wholth_ConsumptionLogArray logs =
            wholth_pages_consumption_log_array(page);

        ASSERT_EQ(0, logs.size);
        ASSERT_EQ(nullptr, logs.data);
    }
}

TEST_F(Test_wholth_pages_consumption_log, when_not_found_by_period)
{
    auto buf = wholth_buffer_ring_pool_element();
    wholth_em_user_locale_id(wtsv("1"), wtsv("1"), buf);

    wholth_Page* page = nullptr;
    auto err = wholth_pages_consumption_log(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_WHOLTH_OK(err);

    wholth_pages_consumption_log_period(
        page, wtsv("1999-02-02T22:22:22"), wtsv("2000-02-02T22:22:22"));

    err = wholth_pages_fetch(page);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::NOT_FOUND, ec) << ec << ec.message();

    const wholth_ConsumptionLogArray nuts =
        wholth_pages_consumption_log_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);
}

TEST_F(Test_wholth_pages_consumption_log, when_requested_page_number_is_too_big)
{
    auto buf = wholth_buffer_ring_pool_element();
    wholth_em_user_locale_id(wtsv("1"), wtsv("1"), buf);

    wholth_Page* page = nullptr;
    auto err = wholth_pages_consumption_log(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    ASSERT_TRUE(
        wholth_pages_skip_to(page, std::numeric_limits<uint64_t>::max()));

    err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_WHOLTH_OK(err);

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::QUERY_PAGE_TOO_BIG, ec)
        << ec << ec.message();

    const wholth_ConsumptionLogArray nuts =
        wholth_pages_consumption_log_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    // not guranteed
    // ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

// Requested offset (page_num*per_page) is too big for int.
TEST_F(Test_wholth_pages_consumption_log, when_rquested_offset_is_to_big)
{
    auto buf = wholth_buffer_ring_pool_element();
    wholth_em_user_locale_id(wtsv("1"), wtsv("1"), buf);

    wholth_Page* page = nullptr;
    auto err = wholth_pages_consumption_log(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    ASSERT_TRUE(
        wholth_pages_skip_to(page, std::numeric_limits<int>::max() / 2));

    err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_WHOLTH_OK(err);

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::QUERY_OFFSET_TOO_BIG, ec)
        << ec << ec.message();

    const wholth_ConsumptionLogArray nuts =
        wholth_pages_consumption_log_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    // not guranteed
    // ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

TEST_F(Test_wholth_pages_consumption_log, when_basic_case)
{
    auto buf = wholth_buffer_ring_pool_element();
    wholth_em_user_locale_id(wtsv("1"), wtsv("1"), buf);

    wholth_Page* page = nullptr;
    auto err = wholth_pages_consumption_log(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    wholth_pages_consumption_log_period(
        page, wtsv("1999-02-02T22:22:22"), wtsv("3000-02-02T22:22:22"));

    ASSERT_TRUE(wholth_pages_skip_to(page, 0));

    err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_WHOLTH_OK(err);

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_ConsumptionLogArray logs =
        wholth_pages_consumption_log_array(page);

    ASSERT_EQ(8, logs.size);
    ASSERT_NE(nullptr, logs.data);

    std::vector<std::vector<std::string_view>> expectations{{
        // 101 * 120 / 100
        {"99990", "1", "101", "2020-01-30T11:00:00", "cl-1-1", "121.2"},
        // 102 * 77 / 100
        {"99991", "2", "102", "2020-01-30T11:00:01", "cl-2-1", "78.54"},
        {"99992", "3", "103", "2025-08-30T01:11:11", "cl-3-1", "0"},
        {"99993", "4", "104", "2025-08-30T05:11:11", "cl-4-1", "0"},
        {"99994", "5", "105", "2025-08-30T11:11:11", "cl-5-1", "0"},
        {"99995", "6", "106", "2025-08-30T13:11:11", "cl-6-1", "0"},
        {"99996", "7", "107", "2025-08-30T18:11:11", "cl-7-1", "0"},
        {"99997", "8", "108", "2300-09-23T00:00:00", "cl-8-1", "0"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(logs.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(logs.data[i].food_id));
        ASSERT_STREQ3(value[2], wfsv(logs.data[i].mass));
        ASSERT_STREQ3(value[3], wfsv(logs.data[i].consumed_at));
        ASSERT_STREQ3(value[4], wfsv(logs.data[i].food_title));
        ASSERT_STREQ3(value[5], wfsv(logs.data[i].nutrient_amount));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 8);
    ASSERT_EQ(wholth_pages_span_size(page), 8);
}

TEST_F(Test_wholth_pages_consumption_log, when_basic_case_and_diff_locale)
{
    auto buf = wholth_buffer_ring_pool_element();
    wholth_em_user_locale_id(wtsv("1"), wtsv("2"), buf);

    wholth_Page* page = nullptr;
    auto err = wholth_pages_consumption_log(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    wholth_pages_consumption_log_period(
        page, wtsv("1999-02-02T22:22:22"), wtsv("3000-02-02T22:22:22"));

    ASSERT_TRUE(wholth_pages_skip_to(page, 0));

    err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_WHOLTH_OK(err);

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_ConsumptionLogArray logs =
        wholth_pages_consumption_log_array(page);

    ASSERT_EQ(8, logs.size);
    ASSERT_NE(nullptr, logs.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"99990", "1", "101", "2020-01-30T11:00:00", "cl-1-2"},
        {"99991", "2", "102", "2020-01-30T11:00:01", "[N/A]"},
        {"99992", "3", "103", "2025-08-30T01:11:11", "cl-3-2"},
        {"99993", "4", "104", "2025-08-30T05:11:11", "cl-4-2"},
        {"99994", "5", "105", "2025-08-30T11:11:11", "cl-5-2"},
        {"99995", "6", "106", "2025-08-30T13:11:11", "cl-6-2"},
        {"99996", "7", "107", "2025-08-30T18:11:11", "cl-7-2"},
        {"99997", "8", "108", "2300-09-23T00:00:00", "[N/A]"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(logs.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(logs.data[i].food_id));
        ASSERT_STREQ3(value[2], wfsv(logs.data[i].mass));
        ASSERT_STREQ3(value[3], wfsv(logs.data[i].consumed_at));
        ASSERT_STREQ3(value[4], wfsv(logs.data[i].food_title));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 8);
    ASSERT_EQ(wholth_pages_span_size(page), 8);
}

TEST_F(Test_wholth_pages_consumption_log, when_small_period)
{
    auto buf = wholth_buffer_ring_pool_element();
    wholth_em_user_locale_id(wtsv("1"), wtsv("1"), buf);

    wholth_Page* page = nullptr;
    auto err = wholth_pages_consumption_log(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    wholth_pages_consumption_log_period(
        page, wtsv("2025-08-30T12:00:00"), wtsv("2025-08-30T19:00:00"));

    ASSERT_TRUE(wholth_pages_skip_to(page, 0));

    err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_WHOLTH_OK(err);

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_ConsumptionLogArray logs =
        wholth_pages_consumption_log_array(page);

    ASSERT_EQ(2, logs.size);
    ASSERT_NE(nullptr, logs.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"99995", "6", "106", "2025-08-30T13:11:11", "cl-6-1"},
        {"99996", "7", "107", "2025-08-30T18:11:11", "cl-7-1"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(logs.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(logs.data[i].food_id));
        ASSERT_STREQ3(value[2], wfsv(logs.data[i].mass));
        ASSERT_STREQ3(value[3], wfsv(logs.data[i].consumed_at));
        ASSERT_STREQ3(value[4], wfsv(logs.data[i].food_title));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 2);
    ASSERT_EQ(wholth_pages_span_size(page), 2);
}

TEST_F(Test_wholth_pages_consumption_log, when_multiple_pages)
{
    auto buf = wholth_buffer_ring_pool_element();
    wholth_em_user_locale_id(wtsv("1"), wtsv("1"), buf);

    {
        wholth_Page* page = nullptr;
        auto err = wholth_pages_consumption_log(&page, 6);
        auto wrap = PageWrap{page};
        ASSERT_WHOLTH_OK(err);

        wholth_pages_consumption_log_period(
            page, wtsv("1999-02-02T22:22:22"), wtsv("3000-02-02T22:22:22"));

        ASSERT_TRUE(wholth_pages_skip_to(page, 0));

        err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_WHOLTH_OK(err);

        err = wholth_pages_fetch(page);

        ASSERT_WHOLTH_OK(err);

        const wholth_ConsumptionLogArray logs =
            wholth_pages_consumption_log_array(page);

        ASSERT_EQ(6, logs.size);
        ASSERT_NE(nullptr, logs.data);

        std::vector<std::vector<std::string_view>> expectations{{
            {"99990", "1", "101", "2020-01-30T11:00:00", "cl-1-1"},
            {"99991", "2", "102", "2020-01-30T11:00:01", "cl-2-1"},
            {"99992", "3", "103", "2025-08-30T01:11:11", "cl-3-1"},
            {"99993", "4", "104", "2025-08-30T05:11:11", "cl-4-1"},
            {"99994", "5", "105", "2025-08-30T11:11:11", "cl-5-1"},
            {"99995", "6", "106", "2025-08-30T13:11:11", "cl-6-1"},
        }};
        for (size_t i = 0; i < expectations.size(); i++)
        {
            const auto& value = expectations[i];

            ASSERT_STREQ3(value[0], wfsv(logs.data[i].id));
            ASSERT_STREQ3(value[1], wfsv(logs.data[i].food_id));
            ASSERT_STREQ3(value[2], wfsv(logs.data[i].mass));
            ASSERT_STREQ3(value[3], wfsv(logs.data[i].consumed_at));
            ASSERT_STREQ3(value[4], wfsv(logs.data[i].food_title));
        }

        ASSERT_EQ(wholth_pages_max(page), 1);
        ASSERT_EQ(wholth_pages_current_page_num(page), 0);
        ASSERT_EQ(wholth_pages_count(page), 8);
        ASSERT_EQ(wholth_pages_span_size(page), 6);
    }

    {
        wholth_Page* page = nullptr;
        auto err = wholth_pages_consumption_log(&page, 6);
        auto wrap = PageWrap{page};
        ASSERT_WHOLTH_OK(err);

        wholth_pages_consumption_log_period(
            page, wtsv("1999-02-02T22:22:22"), wtsv("3000-02-02T22:22:22"));

        ASSERT_TRUE(wholth_pages_skip_to(page, 1));

        err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_WHOLTH_OK(err);

        err = wholth_pages_fetch(page);

        ASSERT_WHOLTH_OK(err);

        const wholth_ConsumptionLogArray logs =
            wholth_pages_consumption_log_array(page);

        ASSERT_EQ(2, logs.size);
        ASSERT_NE(nullptr, logs.data);

        std::vector<std::vector<std::string_view>> expectations{{
            {"99996", "7", "107", "2025-08-30T18:11:11", "cl-7-1"},
            {"99997", "8", "108", "2300-09-23T00:00:00", "cl-8-1"},
        }};
        for (size_t i = 0; i < expectations.size(); i++)
        {
            const auto& value = expectations[i];

            ASSERT_STREQ3(value[0], wfsv(logs.data[i].id));
            ASSERT_STREQ3(value[1], wfsv(logs.data[i].food_id));
            ASSERT_STREQ3(value[2], wfsv(logs.data[i].mass));
            ASSERT_STREQ3(value[3], wfsv(logs.data[i].consumed_at));
            ASSERT_STREQ3(value[4], wfsv(logs.data[i].food_title));
        }

        ASSERT_EQ(wholth_pages_max(page), 1);
        ASSERT_EQ(wholth_pages_current_page_num(page), 1);
        ASSERT_EQ(wholth_pages_count(page), 8);
        ASSERT_EQ(wholth_pages_span_size(page), 2);
    }
}
