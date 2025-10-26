#include "gtest/gtest.h"
#include <array>
#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <type_traits>
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/app_c.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/forward.h"
#include "wholth/c/pages/food.h"
#include "wholth/c/pages/utils.h"
#include "wholth/pages/internal.hpp"

static_assert(nullptr == (void*)NULL);

class Test_wholth_pages_food : public ApplicationAwareTest
{
  protected:
    static void SetUpTestSuite()
    {
        ApplicationAwareTest::SetUpTestSuite();
        auto& con = db::connection();

        astmt(con, "SAVEPOINT Test_wholth_pages_food");

        std::vector<std::vector<
            std::tuple<std::string_view, std::string_view, std::string_view>>>
            deets{{
                // 1
                {
                    {"saltabre", "Essence of salt", "1"},
                    {"Salta", {}, "2"},
                },
                // 2
                {
                    {
                        "Cacaoabra",
                        {},
                        "2",
                    },
                },
                // 3
                {
                    {"Saltabar", {}, "2"},
                },
                // 4
                {
                    {"Salia", {}, "2"},
                    {"Saliagr", "I dont't know", "1"},
                },
                // 5
                {},
                // 6
                {},
            }};

        std::vector<std::string> ids(deets.size(), "");
        for (size_t i = 0; i < deets.size(); i++)
        {
            auto& id = ids[i];
            id = std::to_string(i + 1);

            for (const auto& deet : deets[i])
            {
                astmt(
                    con,
                    fmt::format(
                        "INSERT OR REPLACE INTO food_localisation (food_id, "
                        "locale_id, "
                        "title, description) "
                        "VALUES ({0}, {1}, '{2}', {3})",
                        id,
                        std::get<2>(deet),
                        std::get<0>(deet),
                        std::get<1>(deet).empty()
                            ? fmt::format("'{}'", std::get<1>(deet))
                            : "NULL"));
            }
        }

        std::vector<std::vector<std::string_view>> nutrients{{
            // 1
            {
                "100",
                "22.4",
                "20",
            },
            // 2
            {
                "99",
                "19",
                "20",
            },
            // 3
            {
                "100",
                "20",
                "10.33",
            },
            // 4
            {
                "100",
                "30",
                "39.999",
            },
            // 5
            {
                "10",
                "20",
                "20",
            },
            // 6
            {
                "100",
                "0",
                "50",
            },
        }};

        ASSERT_EQ(nutrients.size(), ids.size());

        std::vector<std::string> nutrient_ids;
        astmt(
            con,
            "SELECT id FROM nutrient ORDER BY position ASC LIMIT 3",
            [&nutrient_ids](sqlw::Statement::ExecArgs e) {
                nutrient_ids.emplace_back(e.column_value);
            });

        for (size_t i = 0; i < nutrients.size(); i++)
        {
            const auto& food_id = ids[i];
            for (size_t j = 0; j < nutrients[i].size(); j++)
            {
                const auto& nutrient_id = nutrient_ids[j];
                const auto& value = nutrients[i][j];
                astmt(
                    con,
                    fmt::format(
                        "INSERT OR REPLACE INTO food_nutrient (food_id, "
                        "nutrient_id, "
                        "value) "
                        "VALUES "
                        "({0}, {1}, {2})",
                        food_id,
                        nutrient_id,
                        value));
            }
        }
        astmt(
            con,
            "INSERT INTO recipe_step (recipe_id,seconds) VALUES "
            "(1, 600) ");
        astmt(
            con,
            "INSERT INTO food (id, created_at) "
            "VALUES (99999, '10-10-2010') ");
    }

    static void TearDownTestSuite()
    {
        astmt(db::connection(), "ROLLBACK TO Test_wholth_pages_food");
    }
};

TEST_F(Test_wholth_pages_food, fetch_when_first_page)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = wholth_pages_food(1, 8);
    const wholth_Error err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const wholth_FoodArray foods = wholth_pages_food_array(page);

    // for (size_t i = 0; i < foods.size; i++)
    // {
    //     std::cout << "id: " << wfsv(foods.data[i].id) << "; "       //
    //               << "title: " << wfsv(foods.data[i].title) << "; " //
    //               << "description: " << wfsv(foods.data[i].description)
    //               << "; " //
    //               << "prep_time: " << wfsv(foods.data[i].preparation_time)
    //               << "; " //
    //               << "top_nut: " << wfsv(foods.data[i].top_nutrient) << '\n';
    // }

    ASSERT_EQ(8, foods.size);
    ASSERT_NE(nullptr, foods.data);

    ASSERT_STREQ2("1", wfsv(foods.data[0].id));
    ASSERT_STREQ2("saltabre", wfsv(foods.data[0].title));
    ASSERT_STREQ2("10m", wfsv(foods.data[0].preparation_time));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[0].top_nutrient));
    ASSERT_EQ(nullptr, foods.data[0].description.data);
    ASSERT_EQ(0, foods.data[0].description.size);

    ASSERT_STREQ2("2", wfsv(foods.data[1].id));
    ASSERT_STREQ2("99.0 KCAL", wfsv(foods.data[1].top_nutrient));

    ASSERT_STREQ2("3", wfsv(foods.data[2].id));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[2].top_nutrient));

    ASSERT_STREQ2("4", wfsv(foods.data[3].id));
    ASSERT_STREQ2("Saliagr", wfsv(foods.data[3].title));
    ASSERT_STREQ2("[N/A]", wfsv(foods.data[3].preparation_time));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[3].top_nutrient));

    ASSERT_STREQ2("5", wfsv(foods.data[4].id));
    ASSERT_STREQ2("10.0 KCAL", wfsv(foods.data[4].top_nutrient));

    ASSERT_STREQ2("6", wfsv(foods.data[5].id));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[5].top_nutrient));

    ASSERT_STREQ2("7", wfsv(foods.data[6].id));

    ASSERT_STREQ2("8", wfsv(foods.data[7].id));

    ASSERT_GT(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 8);
}

TEST_F(Test_wholth_pages_food, fetch_when_second_page)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = wholth_pages_food(2, 8);
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    ASSERT_TRUE(wholth_pages_advance(page, 1));
    const wholth_Error err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const auto foods = wholth_pages_food_array(page);

    ASSERT_EQ(8, foods.size);
    ASSERT_NE(nullptr, foods.data);

    ASSERT_STREQ2("9", wfsv(foods.data[0].id));
    ASSERT_STREQ2("10", wfsv(foods.data[1].id));
    ASSERT_STREQ2("11", wfsv(foods.data[2].id));
    ASSERT_STREQ2("12", wfsv(foods.data[3].id));
    ASSERT_STREQ2("13", wfsv(foods.data[4].id));
    ASSERT_STREQ2("14", wfsv(foods.data[5].id));
    ASSERT_STREQ2("15", wfsv(foods.data[6].id));
    ASSERT_STREQ2("16", wfsv(foods.data[7].id));

    ASSERT_GT(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 1);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 8);
}

TEST_F(Test_wholth_pages_food, fetch_when_first_page_diff_locale)
{
    wholth_user_locale_id(wtsv("2"));
    wholth_Page* page = wholth_pages_food(1, 8);
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    const wholth_Error err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const auto foods = wholth_pages_food_array(page);

    ASSERT_EQ(8, foods.size);
    ASSERT_NE(nullptr, foods.data);

    ASSERT_STREQ2("1", wfsv(foods.data[0].id));
    ASSERT_STREQ2("Salta", wfsv(foods.data[0].title));
    ASSERT_STREQ2("10m", wfsv(foods.data[0].preparation_time));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[0].top_nutrient));
    ASSERT_EQ(nullptr, foods.data[0].description.data);
    ASSERT_EQ(0, foods.data[0].description.size);

    ASSERT_STREQ2("2", wfsv(foods.data[1].id));
    ASSERT_STREQ2("99.0 KCAL", wfsv(foods.data[1].top_nutrient));
    ASSERT_STREQ2("Cacaoabra", wfsv(foods.data[1].title));

    ASSERT_STREQ2("3", wfsv(foods.data[2].id));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[2].top_nutrient));
    ASSERT_STREQ2("Saltabar", wfsv(foods.data[2].title));

    ASSERT_STREQ2("4", wfsv(foods.data[3].id));
    ASSERT_STREQ2("Salia", wfsv(foods.data[3].title));
    ASSERT_STREQ2("[N/A]", wfsv(foods.data[3].preparation_time));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[3].top_nutrient));

    ASSERT_STREQ2("5", wfsv(foods.data[4].id));
    ASSERT_STREQ2("10.0 KCAL", wfsv(foods.data[4].top_nutrient));

    ASSERT_STREQ2("6", wfsv(foods.data[5].id));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[5].top_nutrient));

    ASSERT_STREQ2("7", wfsv(foods.data[6].id));

    ASSERT_STREQ2("8", wfsv(foods.data[7].id));

    ASSERT_GT(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), 8);
}

TEST_F(Test_wholth_pages_food, fetch_should_clamp_on_last_page)
{
    wholth_user_locale_id(wtsv("1"));
    wholth_Page* page = wholth_pages_food(3, 8);
    ASSERT_TRUE(wholth_pages_skip_to(page, 99999));
    const wholth_Error err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const auto foods = wholth_pages_food_array(page);

    ASSERT_LT(foods.size, 8);
    ASSERT_NE(nullptr, foods.data);

    ASSERT_STREQ2("99999", wfsv(foods.data[foods.size - 1].id));

    ASSERT_GT(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), wholth_pages_max(page));
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), foods.size);
}

TEST_F(Test_wholth_pages_food, fetch_when_searched_by_title)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = wholth_pages_food(1, 8);
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    wholth_pages_food_title(page, wtsv("Sal"));
    const wholth_Error err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const auto foods = wholth_pages_food_array(page);

    ASSERT_GE(foods.size, 2);
    ASSERT_NE(nullptr, foods.data);

    ASSERT_STREQ2("1", wfsv(foods.data[0].id));
    ASSERT_STREQ2("saltabre", wfsv(foods.data[0].title));
    ASSERT_STREQ2("10m", wfsv(foods.data[0].preparation_time));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[0].top_nutrient));
    ASSERT_EQ(nullptr, foods.data[0].description.data);
    ASSERT_EQ(0, foods.data[0].description.size);

    ASSERT_STREQ2("4", wfsv(foods.data[1].id));
    ASSERT_STREQ2("Saliagr", wfsv(foods.data[1].title));

    ASSERT_GT(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), foods.size);
}

TEST_F(Test_wholth_pages_food, fetch_when_searched_by_title_and_diff_locale)
{
    wholth_user_locale_id(wtsv("2"));

    wholth_Page* page = wholth_pages_food(99, 8);
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    wholth_pages_food_title(page, wtsv("Sal"));
    const wholth_Error err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const auto foods = wholth_pages_food_array(page);

    ASSERT_GE(foods.size, 3);
    ASSERT_NE(nullptr, foods.data);

    ASSERT_STREQ2("1", wfsv(foods.data[0].id));
    ASSERT_STREQ2("Salta", wfsv(foods.data[0].title));
    ASSERT_STREQ2("10m", wfsv(foods.data[0].preparation_time));
    ASSERT_STREQ2("100.0 KCAL", wfsv(foods.data[0].top_nutrient));
    ASSERT_EQ(nullptr, foods.data[0].description.data);
    ASSERT_EQ(0, foods.data[0].description.size);

    ASSERT_STREQ2("3", wfsv(foods.data[1].id));
    ASSERT_STREQ2("Saltabar", wfsv(foods.data[1].title));

    ASSERT_STREQ2("4", wfsv(foods.data[2].id));
    ASSERT_STREQ2("Salia", wfsv(foods.data[2].title));

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), foods.size);
}

TEST_F(Test_wholth_pages_food, fetch_should_filter_by_ingredients)
{
    auto& con = db::connection();

    astmt(
        con,
        "INSERT INTO recipe_step (recipe_id,seconds) VALUES (2, 150);"
        "INSERT INTO recipe_step_food (recipe_step_id, food_id, "
        "canonical_mass) "
        "  SELECT last_insert_rowid(), fl.food_id, 120 "
        "  FROM food_localisation fl "
        "  WHERE fl.title = 'Saliagr' AND locale_id = 1;"

        "INSERT INTO recipe_step (recipe_id,seconds) VALUES (3, 200);"
        "INSERT INTO recipe_step_food (recipe_step_id, food_id, "
        "canonical_mass) "
        "  SELECT last_insert_rowid(), fl.food_id, 220 "
        "  FROM food_localisation fl"
        "  WHERE fl.title = 'bacon' AND locale_id = 1 "
        "  LIMIT 1;"

        "INSERT INTO recipe_step (recipe_id,seconds) VALUES (4, 300);"
        "INSERT INTO recipe_step_food (recipe_step_id, food_id, "
        "canonical_mass) "
        "  SELECT last_insert_rowid(), fl.food_id, 320 "
        "  FROM food_localisation fl "
        "  WHERE "
        "    (fl.title = 'bacon' OR fl.title = 'Saliagr') "
        "    AND locale_id = 1 "
        "  LIMIT 2");

    // astmt(con, "SELECT * FROM recipe_step_food", [](auto e) {
    //     fmt::print(
    //         fmt::fg(fmt::color::orange),
    //         "{0}: {1}\n",
    //         e.column_name,
    //         e.column_value);
    // });
    // astmt(con, "SELECT * FROM recipe_step", [](auto e) {
    //     fmt::print(
    //         fmt::fg(fmt::color::indian_red),
    //         "{0}: {1}\n",
    //         e.column_name,
    //         e.column_value);
    // });

    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = wholth_pages_food(1, 8);
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    wholth_pages_food_title(page, wholth_StringView_default);
    wholth_pages_food_ingredients(page, wtsv("bacon,Saliagr"));
    const wholth_Error err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const auto foods = wholth_pages_food_array(page);

    ASSERT_GE(foods.size, 8);
    ASSERT_NE(nullptr, foods.data);

    ASSERT_STREQ2("2", wfsv(foods.data[0].id));

    ASSERT_STREQ2("3", wfsv(foods.data[1].id));

    ASSERT_STREQ2("4", wfsv(foods.data[2].id));

    ASSERT_GT(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 3);
    ASSERT_EQ(wholth_pages_span_size(page), 8);
}

// todo move to other file and remove inclue of internal.hpp from here.
TEST_F(Test_wholth_pages_food, prefered_slot_checks)
{
    std::thread t1{[]() {
        wholth_Page* page0 = wholth_pages_food(44, 8);
        wholth_Page* page0_ref = wholth_pages_food(44, 30);
        ASSERT_EQ(page0, page0_ref);
        ASSERT_EQ(
            std::get<wholth::pages::internal::PageType::FOOD>(page0->data).slot,
            44);
        ASSERT_EQ(page0->pagination.per_page(), 8);
    }};

    std::thread t2{[] {
        wholth_Page* page1 = wholth_pages_food(45, 7);
        wholth_Page* page1_ref = wholth_pages_food(45, 40);
        ASSERT_EQ(page1, page1_ref);
        ASSERT_EQ(
            std::get<wholth::pages::internal::PageType::FOOD>(page1->data).slot,
            45);
        ASSERT_EQ(page1->pagination.per_page(), 7);
    }};

    t2.join();
    t1.join();
}
