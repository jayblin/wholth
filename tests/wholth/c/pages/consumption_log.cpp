#include "gtest/gtest.h"
#include <type_traits>
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
            "INSERT OR REPLACE INTO food_localisation "
            " (food_id, locale_id, title) VALUES "
            " (1,      1, 'cl-1-1'),"
            " (1,      2, 'cl-1-2'),"
            " (2,      1, 'cl-2-1'),"
            " (3,      1, 'cl-3-1'),"
            " (3,      2, 'cl-3-2'),"
            " (4,      1, 'cl-4-1'),"
            " (4,      2, 'cl-4-2'),"
            " (5,      1, 'cl-5-1'),"
            " (5,      2, 'cl-5-2'),"
            " (6,      1, 'cl-6-1'),"
            " (6,      2, 'cl-6-2'),"
            " (7,      1, 'cl-7-1'),"
            " (7,      2, 'cl-7-2'),"
            " (8,      1, 'cl-8-1');");
        astmt(
            db::connection(),
            "DELETE FROM food_localisation WHERE food_id IN (2,8) AND "
            "locale_id = 2");
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
        wholth_Page* page = wholth_pages_consumption_log(8, true);
        wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

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
        wholth_Page* page = wholth_pages_consumption_log(8, true);
        wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

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
        wholth_Page* page = wholth_pages_consumption_log(8, false);

        wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

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

    wholth_Page* page = wholth_pages_consumption_log(8, true);

    wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_EQ(wholth_Error_OK.code, err.code)
        << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

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

    wholth_Page* page = wholth_pages_consumption_log(8, true);

    ASSERT_TRUE(
        wholth_pages_skip_to(page, std::numeric_limits<uint64_t>::max()));

    wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_EQ(wholth_Error_OK.code, err.code)
        << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    err = wholth_pages_fetch(page);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

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

    wholth_Page* page = wholth_pages_consumption_log(8, true);

    ASSERT_TRUE(
        wholth_pages_skip_to(page, std::numeric_limits<int>::max() / 2));

    wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_EQ(wholth_Error_OK.code, err.code)
        << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    err = wholth_pages_fetch(page);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

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

    wholth_Page* page = wholth_pages_consumption_log(8, true);

    wholth_pages_consumption_log_period(
        page, wtsv("1999-02-02T22:22:22"), wtsv("3000-02-02T22:22:22"));

    ASSERT_TRUE(wholth_pages_skip_to(page, 0));

    wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_EQ(wholth_Error_OK.code, err.code)
        << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const wholth_ConsumptionLogArray logs =
        wholth_pages_consumption_log_array(page);

    ASSERT_EQ(8, logs.size);
    ASSERT_NE(nullptr, logs.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"99990", "1", "101", "2020-01-30T11:00:00", "cl-1-1"},
        {"99991", "2", "102", "2020-01-30T11:00:01", "cl-2-1"},
        {"99992", "3", "103", "2025-08-30T01:11:11", "cl-3-1"},
        {"99993", "4", "104", "2025-08-30T05:11:11", "cl-4-1"},
        {"99994", "5", "105", "2025-08-30T11:11:11", "cl-5-1"},
        {"99995", "6", "106", "2025-08-30T13:11:11", "cl-6-1"},
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

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 8);
    ASSERT_EQ(wholth_pages_span_size(page), 8);
}

TEST_F(Test_wholth_pages_consumption_log, when_basic_case_and_diff_locale)
{
    auto buf = wholth_buffer_ring_pool_element();
    wholth_em_user_locale_id(wtsv("1"), wtsv("2"), buf);

    wholth_Page* page = wholth_pages_consumption_log(8, true);

    wholth_pages_consumption_log_period(
        page, wtsv("1999-02-02T22:22:22"), wtsv("3000-02-02T22:22:22"));

    ASSERT_TRUE(wholth_pages_skip_to(page, 0));

    wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_EQ(wholth_Error_OK.code, err.code)
        << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

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

    wholth_Page* page = wholth_pages_consumption_log(8, true);

    wholth_pages_consumption_log_period(
        page, wtsv("2025-08-30T12:00:00"), wtsv("2025-08-30T19:00:00"));

    ASSERT_TRUE(wholth_pages_skip_to(page, 0));

    wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
    ASSERT_EQ(wholth_Error_OK.code, err.code)
        << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

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
        wholth_Page* page = wholth_pages_consumption_log(6, true);

        wholth_pages_consumption_log_period(
            page, wtsv("1999-02-02T22:22:22"), wtsv("3000-02-02T22:22:22"));

        ASSERT_TRUE(wholth_pages_skip_to(page, 0));

        wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        err = wholth_pages_fetch(page);

        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

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
        wholth_Page* page = wholth_pages_consumption_log(0, false);

        wholth_pages_consumption_log_period(
            page, wtsv("1999-02-02T22:22:22"), wtsv("3000-02-02T22:22:22"));

        ASSERT_TRUE(wholth_pages_skip_to(page, 1));

        wholth_Error err = wholth_pages_consumption_log_user_id(page, wtsv("1"));
        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        err = wholth_pages_fetch(page);

        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

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
