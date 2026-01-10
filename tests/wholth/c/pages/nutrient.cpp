#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <type_traits>
#include "helpers.hpp"
#include "wholth/c/pages/utils.h"
#include "wholth/pages/code.hpp"
#include "wholth/c/pages/nutrient.h"

static_assert(nullptr == (void*)NULL);

class Test_wholth_pages_nutrient : public ApplicationAwareTest
{
  protected:
    static void SetUpTestSuite()
    {
        ApplicationAwareTest::SetUpTestSuite();
        astmt(db::connection(), "SAVEPOINT Test_wholth_pages_nutrient");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO nutrient (id,unit,position) "
            "VALUES "
            "(1,'mg',0),(2,'mg',1),(3,'mg',2),(4,'mg',3),(5,'mg',4),(6,'mg',5),"
            "(7,'mg',6),(8,'mg',7)");
        astmt(
            db::connection(),
            "DELETE FROM nutrient_localisation_fts5 WHERE 1=1");
        astmt(
            db::connection(),
            "INSERT INTO nutrient_localisation_fts5 (rowid,title) "
            "VALUES "
            "(10001,   'Апетитный'),"
            "(10002,   'Apetitniy'),"
            "(10003,   'C1'),"
            "(10004,   'C2'),"
            "(10005,   'D2'),"
            "(10006,   'E1'),"
            "(10007,   'F1'),"
            "(10008,   'F2'),"
            "(10009,   'G1'),"
            "(10010,   'G2'),"
            "(10011,   'H1'),"
            "(10012,   'H2')");
        astmt(
            db::connection(),
            "INSERT INTO nutrient_localisation "
            "(nutrient_id,locale_id,nl_fts5_rowid) "
            "VALUES "
            "(1,1,10001),"
            "(3,1,10003),"
            "(5,1,10006),"
            "(7,1,10009),"
            "(6,1,10007),"
            "(8,1,10011),"
            "(1,2,10002),"
            "(3,2,10004),"
            "(4,2,10005),"
            "(6,2,10008),"
            "(7,2,10010),"
            "(8,2,10012)");
    }

    static void TearDownTestSuite()
    {
        astmt(db::connection(), "ROLLBACK TO Test_wholth_pages_nutrient");
    }
};

TEST_F(Test_wholth_pages_nutrient, when_basic_case)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_nutrient_locale_id(page, wtsv("1")));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);
    // for (size_t i = 0; i < nuts.size; i++)
    // {
    //     std::cout << wfsv(nuts.data[i].title) << '\n';
    // }

    ASSERT_EQ(7, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"1", "Апетитный", "mg", "0"},
        {"3", "C1", "mg", "2"},
        {"4", "D2", "mg", "3"},
        {"5", "E1", "mg", "4"},
        {"6", "F1", "mg", "5"},
        {"7", "G1", "mg", "6"},
        {"8", "H1", "mg", "7"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].position));
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 7);
    ASSERT_EQ(wholth_pages_span_size(page), 7);
}

TEST_F(Test_wholth_pages_nutrient, when_basic_case_diff_locale)
{
    // wholth_user_locale_id(wtsv("2"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_nutrient_locale_id(page, wtsv("2")));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);
    // for (size_t i = 0; i < nuts.size; i++)
    // {
    //     std::cout << wfsv(nuts.data[i].title) << '\n';
    // }

    ASSERT_EQ(7, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"1", "Apetitniy", "mg", "0"},
        {"3", "C2", "mg", "2"},
        {"4", "D2", "mg", "3"},
        {"5", "E1", "mg", "4"},
        {"6", "F2", "mg", "5"},
        {"7", "G2", "mg", "6"},
        {"8", "H2", "mg", "7"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].position));
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 7);
    ASSERT_EQ(wholth_pages_span_size(page), 7);
}

TEST_F(Test_wholth_pages_nutrient, when_basic_case_with_title)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_nutrient_locale_id(page, wtsv("1")));
    wholth_pages_nutrient_title(page, wtsv("C1"));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);

    ASSERT_EQ(1, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"3", "C1", "mg", "2"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].position));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 1);
    ASSERT_EQ(wholth_pages_span_size(page), 1);
}

TEST_F(Test_wholth_pages_nutrient, when_basic_case_with_title_of_dirrent_locale)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_nutrient_locale_id(page, wtsv("2")));
    ASSERT_WHOLTH_OK(wholth_pages_nutrient_title(page, wtsv("Апетитный")));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);
    // for (size_t i = 0; i < nuts.size; i++)
    // {
    //     std::cout << wfsv(nuts.data[i].title) << '\n';
    // }

    ASSERT_EQ(1, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"1", "Apetitniy", "mg"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].unit));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 1);
    ASSERT_EQ(wholth_pages_span_size(page), 1);
}

TEST_F(Test_wholth_pages_nutrient, when_basic_case_with_title_diff_locale)
{
    // wholth_user_locale_id(wtsv("2"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_nutrient_locale_id(page, wtsv("2")));
    wholth_pages_nutrient_title(page, wtsv("C2"));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);
    // for (size_t i = 0; i < nuts.size; i++)
    // {
    //     std::cout << wfsv(nuts.data[i].title) << '\n';
    // }

    ASSERT_EQ(1, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"3", "C2", "mg", "2"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].position));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 1);
    ASSERT_EQ(wholth_pages_span_size(page), 1);
}

TEST_F(Test_wholth_pages_nutrient, when_not_found)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_nutrient_locale_id(page, wtsv("1")));
    wholth_pages_nutrient_title(page, wtsv("AAApetitniy"));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::NOT_FOUND, ec) << ec << ec.message();

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

TEST_F(Test_wholth_pages_nutrient, when_requested_page_number_is_to_big)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    wholth_pages_skip_to(page, std::numeric_limits<uint64_t>::max());

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::QUERY_PAGE_TOO_BIG, ec)
        << ec << ec.message();

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    // not guranteed
    // ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

// Requested offset (page_num*per_page) is too big for int.
TEST_F(Test_wholth_pages_nutrient, when_rquested_offset_is_to_big)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    wholth_pages_skip_to(page, std::numeric_limits<int>::max() / 2);

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::QUERY_OFFSET_TOO_BIG, ec)
        << ec << ec.message();

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    // not guranteed
    // ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

TEST_F(Test_wholth_pages_nutrient, when_second_page)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 4);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_nutrient_locale_id(page, wtsv("1")));
    wholth_pages_skip_to(page, 1);

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);
    // for (size_t i = 0; i < nuts.size; i++)
    // {
    //     std::cout << wfsv(nuts.data[i].title) << '\n';
    // }

    ASSERT_EQ(3, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"6", "F1", "mg", "5"},
        {"7", "G1", "mg", "6"},
        {"8", "H1", "mg", "7"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].position));
    }

    ASSERT_GE(wholth_pages_max(page), 1);
    ASSERT_EQ(wholth_pages_current_page_num(page), 1);
    ASSERT_GE(wholth_pages_count(page), 7);
    ASSERT_EQ(wholth_pages_span_size(page), 3);
}

TEST_F(
    Test_wholth_pages_nutrient,
    when_basic_case_with_titles_separated_by_coma)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_nutrient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_nutrient_locale_id(page, wtsv("1")));
    wholth_pages_nutrient_title(page, wtsv("C1,\" Апетитный\",H2"));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_NutrientArray nuts = wholth_pages_nutrient_array(page);

    ASSERT_EQ(3, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"1", "Апетитный", "mg", "0"},
        {"3", "C1", "mg", "2"},
        {"8", "H1", "mg", "7"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].position));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 3);
    ASSERT_EQ(wholth_pages_span_size(page), 3);
}
