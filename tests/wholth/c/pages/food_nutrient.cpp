#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <type_traits>
#include "helpers.hpp"
#include "wholth/c/app.h"
#include "wholth/c/entity/nutrient.h"
#include "wholth/c/pages/utils.h"
#include "wholth/entity_manager/food.hpp"
#include "wholth/pages/code.hpp"
#include "wholth/c/pages/food_nutrient.h"

static_assert(nullptr == (void*)NULL);

class Test_wholth_pages_food_nutrient : public ApplicationAwareTest
{
  protected:
    static void SetUpTestSuite()
    {
        ApplicationAwareTest::SetUpTestSuite();
        astmt(db::connection(), "SAVEPOINT Test_wholth_pages_food_nutrient");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO nutrient (id,unit,position) "
            "VALUES "
            "(1,'mg',0),(2,'mg',1),(3,'mg',2),(4,'mg',3),(5,'mg',4),(6,'mg',5),"
            "(7,'mg',6),(8,'mg',7)");
        astmt(
            db::connection(),
            "INSERT INTO nutrient_localisation (nutrient_id,locale_id,title) "
            "VALUES "
            "(1,1,'A1'),"
            "(1,2,'A2'),"
            // "(2,1,'B1'),"
            // "(2,2,'B2'),"
            "(3,1,'C1'),"
            "(3,2,'C2'),"
            // "(4,1,'D1'),"
            "(4,2,'D2'),"
            "(5,1,'E1'),"
            // "(5,2,'E2'),"
            "(6,1,'F1'),"
            "(6,2,'F2'),"
            "(7,1,'G1'),"
            "(7,2,'G2'),"
            "(8,1,'H1'),"
            "(8,2,'H2')");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO food_nutrient (food_id, nutrient_id, "
            "value) "
            "VALUES "
            "(1,1,10),(1,2,20),(1,3,30),(1,4,40),(1,5,50),"
            "(1,6,60),(1,7,70),(1,8,80)");
    }

    static void TearDownTestSuite()
    {
        astmt(db::connection(), "ROLLBACK TO Test_wholth_pages_food_nutrient");
    }
};

// should be executed first
TEST_F(Test_wholth_pages_food_nutrient, when_empty_food_id)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 8);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::FOOD_INVALID_ID, ec)
        << ec << ec.message();

    const wholth_FoodNutrientArray nuts = wholth_pages_food_nutrient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

TEST_F(Test_wholth_pages_food_nutrient, when_invalid_food_id)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 8);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    std::vector<std::string_view> ids{{
        {},
        "",
        "-1",
        "3a",
        "as",
    }};

    for (auto id : ids)
    {
        wholth_pages_food_nutrient_food_id(page, wtsv(id));
        err = wholth_pages_fetch(page);

        ASSERT_WHOLTH_NOK(err);

        std::error_code ec = wholth::entity_manager::food::Code(err.code);
        ASSERT_EQ(wholth::entity_manager::food::Code::FOOD_INVALID_ID, ec)
            << ec << ec.message();

        const wholth_FoodNutrientArray nuts =
            wholth_pages_food_nutrient_array(page);

        ASSERT_EQ(0, nuts.size);
        ASSERT_EQ(nullptr, nuts.data);

        ASSERT_EQ(wholth_pages_max(page), 0);
        ASSERT_EQ(wholth_pages_current_page_num(page), 0);
        ASSERT_EQ(wholth_pages_count(page), 0);
        ASSERT_EQ(wholth_pages_span_size(page), 0);
    }
}

TEST_F(Test_wholth_pages_food_nutrient, when_basic_case)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 8);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    wholth_pages_food_nutrient_food_id(page, wtsv("1"));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::OK, ec) << ec << ec.message();

    const wholth_FoodNutrientArray nuts = wholth_pages_food_nutrient_array(page);

    ASSERT_EQ(8, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"1", "A1", "10", "mg", "0"},
        {"2", "[N/A]", "20", "mg", "1"},
        {"3", "C1", "30", "mg", "2"},
        {"4", "[N/A]", "40", "mg", "3"},
        {"5", "E1", "50", "mg", "4"},
        {"6", "F1", "60", "mg", "5"},
        {"7", "G1", "70", "mg", "6"},
        {"8", "H1", "80", "mg", "7"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].value));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[4], wfsv(nuts.data[i].position));
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 8);
    ASSERT_EQ(wholth_pages_span_size(page), 8);
}

TEST_F(Test_wholth_pages_food_nutrient, when_basic_case_diff_locale)
{
    wholth_user_locale_id(wtsv("2"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 8);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    wholth_pages_food_nutrient_food_id(page, wtsv("1"));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::OK, ec) << ec << ec.message();

    const wholth_FoodNutrientArray nuts = wholth_pages_food_nutrient_array(page);

    ASSERT_EQ(8, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"1", "A2", "10", "mg", "0"},
        {"2", "[N/A]", "20", "mg", "1"},
        {"3", "C2", "30", "mg", "2"},
        {"4", "D2", "40", "mg", "3"},
        {"5", "[N/A]", "50", "mg", "4"},
        {"6", "F2", "60", "mg", "5"},
        {"7", "G2", "70", "mg", "6"},
        {"8", "H2", "80", "mg", "7"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].value));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[4], wfsv(nuts.data[i].position));
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 8);
    ASSERT_EQ(wholth_pages_span_size(page), 8);
}

TEST_F(Test_wholth_pages_food_nutrient, when_basic_case_with_title)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 8);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    wholth_pages_food_nutrient_food_id(page, wtsv("1"));
    wholth_pages_food_nutrient_title(page, wtsv("C1"));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::OK, ec) << ec << ec.message();

    const wholth_FoodNutrientArray nuts = wholth_pages_food_nutrient_array(page);

    ASSERT_EQ(1, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"3", "C1", "30", "mg", "2"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].value));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[4], wfsv(nuts.data[i].position));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 1);
    ASSERT_EQ(wholth_pages_span_size(page), 1);
}

TEST_F(Test_wholth_pages_food_nutrient, when_basic_case_with_title_diff_locale)
{
    wholth_user_locale_id(wtsv("2"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 8);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    wholth_pages_food_nutrient_food_id(page, wtsv("1"));
    wholth_pages_food_nutrient_title(page, wtsv("C2"));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::OK, ec) << ec << ec.message();

    const wholth_FoodNutrientArray nuts = wholth_pages_food_nutrient_array(page);

    ASSERT_EQ(1, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"3", "C2", "30", "mg", "2"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].value));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[4], wfsv(nuts.data[i].position));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GE(wholth_pages_count(page), 1);
    ASSERT_EQ(wholth_pages_span_size(page), 1);
}

TEST_F(Test_wholth_pages_food_nutrient, when_not_found)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 8);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    wholth_pages_food_nutrient_food_id(page, wtsv("1"));
    wholth_pages_food_nutrient_title(page, wtsv("AAA2"));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::NOT_FOUND, ec) << ec << ec.message();

    const wholth_FoodNutrientArray nuts = wholth_pages_food_nutrient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

TEST_F(Test_wholth_pages_food_nutrient, when_requested_page_number_is_to_big)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 8);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    wholth_pages_food_nutrient_food_id(page, wtsv("1"));

    wholth_pages_skip_to(page, std::numeric_limits<uint64_t>::max());

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::QUERY_PAGE_TOO_BIG, ec)
        << ec << ec.message();

    const wholth_FoodNutrientArray nuts = wholth_pages_food_nutrient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    // not guranteed
    // ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

// Requested offset (page_num*per_page) is too big for int.
TEST_F(Test_wholth_pages_food_nutrient, when_rquested_offset_is_to_big)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 8);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    wholth_pages_food_nutrient_food_id(page, wtsv("1"));

    wholth_pages_skip_to(page, std::numeric_limits<int>::max() / 2);

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::QUERY_OFFSET_TOO_BIG, ec)
        << ec << ec.message();

    const wholth_FoodNutrientArray nuts = wholth_pages_food_nutrient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    // not guranteed
    // ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

TEST_F(Test_wholth_pages_food_nutrient, when_second_page)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food_nutrient(&page, 4);
        auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    wholth_pages_food_nutrient_food_id(page, wtsv("1"));
    wholth_pages_skip_to(page, 1);

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::OK, ec) << ec << ec.message();

    const wholth_FoodNutrientArray nuts = wholth_pages_food_nutrient_array(page);

    ASSERT_EQ(4, nuts.size);
    ASSERT_NE(nullptr, nuts.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"5", "E1", "50", "mg", "4"},
        {"6", "F1", "60", "mg", "5"},
        {"7", "G1", "70", "mg", "6"},
        {"8", "H1", "80", "mg", "7"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(nuts.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(nuts.data[i].title));
        ASSERT_STREQ3(value[2], wfsv(nuts.data[i].value));
        ASSERT_STREQ3(value[3], wfsv(nuts.data[i].unit));
        ASSERT_STREQ3(value[4], wfsv(nuts.data[i].position));
    }

    ASSERT_GE(wholth_pages_max(page), 1);
    ASSERT_EQ(wholth_pages_current_page_num(page), 1);
    ASSERT_GE(wholth_pages_count(page), 8);
    ASSERT_EQ(wholth_pages_span_size(page), 4);
}
