#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <type_traits>
#include "helpers.hpp"
#include "wholth/c/pages/utils.h"
#include "wholth/entity_manager/food.hpp"
#include "wholth/pages/code.hpp"
#include "wholth/c/pages/ingredient.h"

static_assert(nullptr == (void*)NULL);

class Test_wholth_pages_ingredient : public ApplicationAwareTest
{
  protected:
    static void SetUpTestSuite()
    {
        ApplicationAwareTest::SetUpTestSuite();
        astmt(db::connection(), "SAVEPOINT Test_wholth_pages_ingredient");

        astmt(db::connection(), "DELETE FROM food_localisation_fts5 WHERE 1=1");
        astmt(db::connection(), "DELETE FROM food_localisation WHERE 1=1");

        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO food (id, created_at) VALUES "
            " (999991, '2025-08-30T11:11:11'),"
            " (999992, '2025-08-30T11:11:11')");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO food_localisation_fts5 "
            " (rowid, title) VALUES "
            " (1,      'twpi-1-1'),"
            " (2,      'twpi-1-2'),"
            " (3,      'twpi-2-1'),"
            " (4,      'twpi-2-2'),"
            " (5,      'twpi-3-1'),"
            " (6,      'twpi-3-2'),"
            " (7,      'twpi-4-1'),"
            " (8,      'twpi-4-2'),"
            " (9,      'twpi-5-1'),"
            " (10,     'twpi-5-2'),"
            " (11,     'twpi-6-1'),"
            " (12,     'twpi-6-2'),"
            " (13,     'twpi-7-1'),"
            " (14,     'twpi-7-2'),"
            " (15,     'a reicpe yea'),"
            " (16,     'an ingredient yea')");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO food_localisation "
            " (food_id, locale_id, fl_fts5_rowid) VALUES "
            " (1,      1, 1),"
            " (1,      2, 2),"
            " (2,      1, 3),"
            " (2,      2, 4),"
            " (3,      1, 5),"
            " (3,      2, 6),"
            " (4,      1, 7),"
            " (4,      2, 8),"
            " (5,      1, 9),"
            " (5,      2, 10),"
            " (6,      1, 11),"
            " (6,      2, 12),"
            " (7,      1, 13),"
            " (7,      2, 14),"
            " (999991, 1, 15),"
            " (999992, 1, 16)");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO recipe_step "
            " (id,      recipe_id, priority, seconds, ingredients_mass)"
            " VALUES "
            " (1111110, 5,         0,        5,        100),"
            " (1111111, 999991,    0,        400,      200),"
            " (1111112, 999991,    1,        600,      300)");
        astmt(
            db::connection(),
            "INSERT INTO recipe_step_food "
            " (id, recipe_step_id, food_id, canonical_mass) VALUES "
            " (777770, 1111110, 6,      600),"
            " (777771, 1111111, 1,      100),"
            " (777772, 1111111, 3,      300),"
            " (777773, 1111111, 4,      400),"
            " (777774, 1111111, 5,      500),"
            " (777775, 1111111, 999992, 900),"
            " (777776, 1111112, 2,      200)");
    }

    static void TearDownTestSuite()
    {
        astmt(db::connection(), "ROLLBACK TO Test_wholth_pages_ingredient");
    }
};

// should br executed first in this group
TEST_F(Test_wholth_pages_ingredient, when_no_food_id)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::FOOD_INVALID_ID, ec)
        << ec << ec.message();

    const wholth_IngredientArray ings = wholth_pages_ingredient_array(page);

    ASSERT_EQ(0, ings.size);
    ASSERT_EQ(nullptr, ings.data);
}

TEST_F(Test_wholth_pages_ingredient, when_not_found_by_id)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_food_id(page, wtsv("9999999")));
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_locale_id(page, wtsv("1")));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::NOT_FOUND, ec) << ec << ec.message();

    const wholth_IngredientArray nuts = wholth_pages_ingredient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);
}

TEST_F(Test_wholth_pages_ingredient, when_not_found_by_title)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_food_id(page, wtsv("999991")));
    ASSERT_WHOLTH_OK(
        wholth_pages_ingredient_title(page, wtsv("\"HI_ij19(_)((33sdf\"")));
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_locale_id(page, wtsv("1")));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);
    // ASSERT_WHOLTH_OK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::NOT_FOUND, ec) << ec << ec.message();

    const wholth_IngredientArray nuts = wholth_pages_ingredient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);
}

TEST_F(Test_wholth_pages_ingredient, when_requested_page_number_is_too_big)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    ASSERT_TRUE(
        wholth_pages_skip_to(page, std::numeric_limits<uint64_t>::max()));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::QUERY_PAGE_TOO_BIG, ec)
        << ec << ec.message();

    const wholth_IngredientArray nuts = wholth_pages_ingredient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    // not guranteed
    // ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

// Requested offset (page_num*per_page) is too big for int.
TEST_F(Test_wholth_pages_ingredient, when_rquested_offset_is_to_big)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);

    ASSERT_TRUE(
        wholth_pages_skip_to(page, std::numeric_limits<int>::max() / 2));

    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_NOK(err);

    std::error_code ec = wholth::pages::Code(err.code);
    ASSERT_EQ(wholth::pages::Code::QUERY_OFFSET_TOO_BIG, ec)
        << ec << ec.message();

    const wholth_IngredientArray nuts = wholth_pages_ingredient_array(page);

    ASSERT_EQ(0, nuts.size);
    ASSERT_EQ(nullptr, nuts.data);

    ASSERT_EQ(wholth_pages_max(page), 0);
    // not guranteed
    // ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 0);
}

TEST_F(Test_wholth_pages_ingredient, when_basic_case)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_food_id(page, wtsv("999991")));
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_locale_id(page, wtsv("1")));
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_IngredientArray ings = wholth_pages_ingredient_array(page);

    ASSERT_EQ(6, ings.size);
    ASSERT_NE(nullptr, ings.data);

    std::vector<std::vector<std::string_view>> expectations{{
        // id       food_id     food_title        c_mass icnt imass
        {"777775", "999992", "an ingredient yea", "900", "0", ""},
        {"777771", "1", "twpi-1-1", "100", "0", ""},
        {"777776", "2", "twpi-2-1", "200", "0", ""},
        {"777772", "3", "twpi-3-1", "300", "0", ""},
        {"777773", "4", "twpi-4-1", "400", "0", ""},
        {"777774", "5", "twpi-5-1", "500", "1", "100"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(ings.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(ings.data[i].food_id));
        ASSERT_STREQ3(value[2], wfsv(ings.data[i].food_title));
        ASSERT_STREQ3(value[3], wfsv(ings.data[i].canonical_mass_g));
        ASSERT_STREQ3(value[4], wfsv(ings.data[i].ingredient_count));
        ASSERT_STREQ3(value[5], wfsv(ings.data[i].ingredients_mass_g));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 6);
    ASSERT_EQ(wholth_pages_span_size(page), 6);
}

TEST_F(Test_wholth_pages_ingredient, when_basic_case_and_diff_locale)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_food_id(page, wtsv("999991")));
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_locale_id(page, wtsv("2")));
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_IngredientArray ings = wholth_pages_ingredient_array(page);

    ASSERT_EQ(6, ings.size);
    ASSERT_NE(nullptr, ings.data);

    std::vector<std::vector<std::string_view>> expectations{{
        // id       food_id     food_title        c_mass icnt imass
        {"777775", "999992", "an ingredient yea", "900", "0", "500"},
        {"777771", "1", "twpi-1-2", "100", "0", ""},
        {"777776", "2", "twpi-2-2", "200", "0", ""},
        {"777772", "3", "twpi-3-2", "300", "0", ""},
        {"777773", "4", "twpi-4-2", "400", "0", ""},
        {"777774", "5", "twpi-5-2", "500", "1", "100"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(ings.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(ings.data[i].food_id));
        ASSERT_STREQ3(value[2], wfsv(ings.data[i].food_title));
        ASSERT_STREQ3(value[3], wfsv(ings.data[i].canonical_mass_g));
        ASSERT_STREQ3(value[4], wfsv(ings.data[i].ingredient_count));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 6);
    ASSERT_EQ(wholth_pages_span_size(page), 6);
}

TEST_F(Test_wholth_pages_ingredient, when_searched_by_title)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_food_id(page, wtsv("999991")));
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_title(page, wtsv("twpi")));
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_locale_id(page, wtsv("1")));
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_IngredientArray ings = wholth_pages_ingredient_array(page);

    ASSERT_EQ(5, ings.size);
    ASSERT_NE(nullptr, ings.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"777771", "1", "twpi-1-1", "100", "0"},
        {"777776", "2", "twpi-2-1", "200", "0"},
        {"777772", "3", "twpi-3-1", "300", "0"},
        {"777773", "4", "twpi-4-1", "400", "0"},
        {"777774", "5", "twpi-5-1", "500", "1"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(ings.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(ings.data[i].food_id));
        ASSERT_STREQ3(value[2], wfsv(ings.data[i].food_title));
        ASSERT_STREQ3(value[3], wfsv(ings.data[i].canonical_mass_g));
        ASSERT_STREQ3(value[4], wfsv(ings.data[i].ingredient_count));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 5);
    ASSERT_EQ(wholth_pages_span_size(page), 5);
}

TEST_F(Test_wholth_pages_ingredient, when_searched_by_title_but_in_diff_locale)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_food_id(page, wtsv("999991")));
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_title(page, wtsv("twpi")));
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_locale_id(page, wtsv("2")));
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_IngredientArray ings = wholth_pages_ingredient_array(page);

    ASSERT_EQ(5, ings.size);
    ASSERT_NE(nullptr, ings.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"777771", "1", "twpi-1-2", "100", "0"},
        {"777776", "2", "twpi-2-2", "200", "0"},
        {"777772", "3", "twpi-3-2", "300", "0"},
        {"777773", "4", "twpi-4-2", "400", "0"},
        {"777774", "5", "twpi-5-2", "500", "1"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(ings.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(ings.data[i].food_id));
        ASSERT_STREQ3(value[2], wfsv(ings.data[i].food_title));
        ASSERT_STREQ3(value[3], wfsv(ings.data[i].canonical_mass_g));
        ASSERT_STREQ3(value[4], wfsv(ings.data[i].ingredient_count));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 5);
    ASSERT_EQ(wholth_pages_span_size(page), 5);
}

TEST_F(Test_wholth_pages_ingredient, when_basic_case_second_page)
{
    {
        wholth_Page* page = nullptr;
        auto err = wholth_pages_ingredient(&page, 3);
        auto wrap = PageWrap{page};
        ASSERT_WHOLTH_OK(err);
        ASSERT_WHOLTH_OK(wholth_pages_ingredient_food_id(page, wtsv("999991")));
        ASSERT_WHOLTH_OK(wholth_pages_ingredient_locale_id(page, wtsv("1")));
        ASSERT_TRUE(wholth_pages_skip_to(page, 0));
        err = wholth_pages_fetch(page);

        ASSERT_WHOLTH_OK(err);

        const wholth_IngredientArray ings = wholth_pages_ingredient_array(page);

        ASSERT_EQ(3, ings.size);
        ASSERT_NE(nullptr, ings.data);

        std::vector<std::vector<std::string_view>> expectations{{
            {"999992", "an ingredient yea", "900", "0"},
            {"1", "twpi-1-1", "100", "0"},
            {"2", "twpi-2-1", "200", "0"},
        }};
        for (size_t i = 0; i < expectations.size(); i++)
        {
            const auto& value = expectations[i];

            ASSERT_STREQ3(value[0], wfsv(ings.data[i].food_id));
            ASSERT_STREQ3(value[1], wfsv(ings.data[i].food_title));
            ASSERT_STREQ3(value[2], wfsv(ings.data[i].canonical_mass_g));
            ASSERT_STREQ3(value[3], wfsv(ings.data[i].ingredient_count));
        }

        ASSERT_EQ(wholth_pages_max(page), 1);
        ASSERT_EQ(wholth_pages_current_page_num(page), 0);
        ASSERT_EQ(wholth_pages_count(page), 6);
        ASSERT_EQ(wholth_pages_span_size(page), 3);
    }

    {
        wholth_Page* page = nullptr;
        auto err = wholth_pages_ingredient(&page, 3);
        auto wrap = PageWrap{page};
        ASSERT_WHOLTH_OK(err);
        ASSERT_WHOLTH_OK(wholth_pages_ingredient_food_id(page, wtsv("999991")));
        ASSERT_WHOLTH_OK(wholth_pages_ingredient_locale_id(page, wtsv("1")));
        ASSERT_TRUE(wholth_pages_advance(page, 1));
        err = wholth_pages_fetch(page);

        ASSERT_WHOLTH_OK(err);

        const wholth_IngredientArray ings = wholth_pages_ingredient_array(page);

        ASSERT_EQ(3, ings.size);
        ASSERT_NE(nullptr, ings.data);

        std::vector<std::vector<std::string_view>> expectations{{
            {"777772", "3", "twpi-3-1", "300", "0"},
            {"777773", "4", "twpi-4-1", "400", "0"},
            {"777774", "5", "twpi-5-1", "500", "1"},
        }};
        for (size_t i = 0; i < expectations.size(); i++)
        {
            const auto& value = expectations[i];

            ASSERT_STREQ3(value[0], wfsv(ings.data[i].id));
            ASSERT_STREQ3(value[1], wfsv(ings.data[i].food_id));
            ASSERT_STREQ3(value[2], wfsv(ings.data[i].food_title));
            ASSERT_STREQ3(value[3], wfsv(ings.data[i].canonical_mass_g));
            ASSERT_STREQ3(value[4], wfsv(ings.data[i].ingredient_count));
        }

        ASSERT_EQ(wholth_pages_max(page), 1);
        ASSERT_EQ(wholth_pages_current_page_num(page), 1);
        ASSERT_EQ(wholth_pages_count(page), 6);
        ASSERT_EQ(wholth_pages_span_size(page), 3);
    }
}

TEST_F(Test_wholth_pages_ingredient, when_searched_by_titles_separated_by_comas)
{
    wholth_Page* page = nullptr;
    auto err = wholth_pages_ingredient(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_food_id(page, wtsv("999991")));
    ASSERT_WHOLTH_OK(
        wholth_pages_ingredient_title(page, wtsv("\"twpi-1\", \"twpi-2\"")));
    ASSERT_WHOLTH_OK(wholth_pages_ingredient_locale_id(page, wtsv("1")));
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const wholth_IngredientArray ings = wholth_pages_ingredient_array(page);

    ASSERT_EQ(2, ings.size);
    ASSERT_NE(nullptr, ings.data);

    std::vector<std::vector<std::string_view>> expectations{{
        {"777771", "1", "twpi-1-1", "100", "0"},
        {"777776", "2", "twpi-2-1", "200", "0"},
    }};
    for (size_t i = 0; i < expectations.size(); i++)
    {
        const auto& value = expectations[i];

        ASSERT_STREQ3(value[0], wfsv(ings.data[i].id));
        ASSERT_STREQ3(value[1], wfsv(ings.data[i].food_id));
        ASSERT_STREQ3(value[2], wfsv(ings.data[i].food_title));
        ASSERT_STREQ3(value[3], wfsv(ings.data[i].canonical_mass_g));
        ASSERT_STREQ3(value[4], wfsv(ings.data[i].ingredient_count));
    }

    ASSERT_EQ(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_EQ(wholth_pages_count(page), 2);
    ASSERT_EQ(wholth_pages_span_size(page), 2);
}
