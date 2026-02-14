#include "gtest/gtest.h"
#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/entity/food.h"
#include "wholth/c/pages/food.h"
#include "wholth/c/pages/utils.h"

static_assert(nullptr == (void*)NULL);

class Test_wholth_pages_food : public ApplicationAwareTest
{
  protected:
    static void SetUpTestSuite()
    {
        ApplicationAwareTest::SetUpTestSuite();
        auto& con = db::connection();

        astmt(con, "SAVEPOINT Test_wholth_pages_food");

        astmt(db::connection(), "DELETE FROM food_localisation_fts5 WHERE 1=1");
        astmt(db::connection(), "DELETE FROM food_localisation WHERE 1=1");

        //   | locale 1         | locale 2
        // --|------------------|----------
        // 1 | saltabre         | Salta
        // 2 | NULL             | Какаобра
        // 3 | NULL             | Saltabar
        // 4 | Saliagr          | Salia
        // 5 | NULL             | Hello there
        // 6 | Генерал Кеноби!  | General Kenobi!
        // 7 | whatever         | NULL
        astmt(
            con,
            "INSERT OR REPLACE INTO food_localisation_fts5 "
            "(rowid,title,description) "
            "VALUES "
            "(1, 'saltabre', 'Essence of salt'),"
            "(2, 'Salta', NULL),"
            "(3, 'Какаобра', NULL),"
            "(4, 'Saltabar', NULL),"
            "(5, 'Salia', NULL),"
            "(6, 'Saliagr', 'I dont''t know'),"
            "(7, 'Hello there', NULL),"
            "(8, 'Генерал Кеноби!', NULL),"
            "(9, 'whatever', NULL),"
            "(10, 'General Kenobi!', NULL);"
            "INSERT OR REPLACE INTO food_localisation "
            "(food_id,locale_id,fl_fts5_rowid) "
            "VALUES "
            "(1,1,1),"
            "(1,2,2),"
            "(2,2,3),"
            "(3,2,4),"
            "(4,1,6),"
            "(4,2,5),"
            "(5,2,7),"
            "(6,1,8),"
            "(6,2,10),"
            "(7,1,9)");

        // size_t j = 0;
        // astmt(
        //     con,
        //     "SELECT fl_fts5.rowid, fl_fts5.title, fl.*"
        //     "FROM food_localisation_fts5 fl_fts5 "
        //     "INNER JOIN food_localisation fl "
        //     " ON fl.fl_fts5_rowid = fl_fts5.rowid "
        //     "ORDER BY fl.food_id ASC, fl.locale_id ASC",
        //     [&j](auto e) {
        //         if (0 == j % e.column_count)
        //         {
        //             std::cout << "------\n";
        //         }
        //         j++;
        //         std::cout << e.column_name << ": " << e.column_value << '\n';
        //     });

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

        std::vector<std::string> nutrient_ids;
        astmt(
            con,
            "SELECT id FROM nutrient ORDER BY position ASC LIMIT 3",
            [&nutrient_ids](sqlw::Statement::ExecArgs e) {
                nutrient_ids.emplace_back(e.column_value);
            });

        ASSERT_EQ(3, nutrient_ids.size());

        astmt(
            con,
            fmt::format(
                "INSERT OR REPLACE INTO food_nutrient "
                "(food_id,nutrient_id,value) "
                "VALUES "
                "(1, {0}, 100),"
                "(1, {1}, 22.4),"
                "(1, {2}, 20),"
                "(2, {0}, 99),"
                "(2, {1}, 10),"
                "(2, {2}, 20),"
                "(3, {0}, 100),"
                "(3, {1}, 20),"
                "(3, {2}, 10.33),"
                "(4, {0}, 100),"
                "(4, {1}, 30),"
                "(4, {2}, 39.999),"
                "(5, {0}, 10),"
                "(5, {1}, 20),"
                "(5, {2}, 20),"
                "(6, {0}, 100),"
                "(6, {1}, 0),"
                "(6, {2}, 50)",
                nutrient_ids[0],
                nutrient_ids[1],
                nutrient_ids[2]));

        astmt(
            con,
            "INSERT INTO recipe_step (recipe_id,seconds,ingredients_mass) VALUES "
            "(1, 600, 61) ");
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
    wholth_Page* page = nullptr;
    auto err = wholth_pages_food(&page, 8);

    auto wrap = PageWrap{page};

    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("1")));
    ASSERT_WHOLTH_OK(wholth_pages_fetch(page));

    const wholth_FoodArray foods = wholth_pages_food_array(page);

    // for (size_t i = 0; i < foods.size; i++)
    // {
    //     std::cout << "-------------\n"              //
    //               << wfsv(foods.data[i].id) << '\n' //
    //               << wfsv(foods.data[i].title) << '\n'
    //               << wfsv(foods.data[i].preparation_time) << '\n'
    //               << wfsv(foods.data[i].top_nutrient) << '\n';
    // }

    using sv = std::string_view;
    std::vector<std::tuple<sv, sv, sv, sv, sv>> expected_foods{{
        {"5", "Hello there", "10.0 KCAL", "", "0"},
        {"4", "Saliagr", "100.0 KCAL", "", "0"},
        {"3", "Saltabar", "100.0 KCAL", "", "0"},
        {"1", "saltabre", "100.0 KCAL", "10m", "61"},
        {"7", "whatever", "[N/A]", "", "0"},
        {"6", "Генерал Кеноби!", "100.0 KCAL", "", "0"},
        {"2", "Какаобра", "99.0 KCAL", "", "0"},
    }};

    ASSERT_EQ(expected_foods.size(), foods.size);
    ASSERT_NE(nullptr, foods.data);

    size_t i = 0;
    for (const auto& expected : expected_foods)
    {
        ASSERT_STREQ3(std::get<0>(expected), wfsv(foods.data[i].id));
        ASSERT_STREQ3(std::get<1>(expected), wfsv(foods.data[i].title));
        ASSERT_STREQ3(std::get<2>(expected), wfsv(foods.data[i].top_nutrient));
        ASSERT_STREQ3(
            std::get<3>(expected), wfsv(foods.data[i].preparation_time));
        ASSERT_STREQ3(
            std::get<4>(expected), wfsv(foods.data[i].ingredients_mass_g));
        i++;
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), expected_foods.size());
}

TEST_F(Test_wholth_pages_food, fetch_when_second_page)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food(&page, 4);

    auto wrap = PageWrap{page};

    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("1")));
    ASSERT_TRUE(wholth_pages_skip_to(page, 1));
    ASSERT_WHOLTH_OK(wholth_pages_fetch(page));

    const wholth_FoodArray foods = wholth_pages_food_array(page);

    // for (size_t i = 0; i < foods.size; i++)
    // {
    //     std::cout << "-------------\n"              //
    //               << wfsv(foods.data[i].id) << '\n' //
    //               << wfsv(foods.data[i].title) << '\n'
    //               << wfsv(foods.data[i].preparation_time) << '\n'
    //               << wfsv(foods.data[i].top_nutrient) << '\n';
    // }

    using sv = std::string_view;
    std::vector<std::tuple<sv, sv, sv, sv>> expected_foods{{
        // {"5", "Hello there", "10.0 KCAL", ""},
        // {"4", "Saliagr", "100.0 KCAL", ""},
        // {"3", "Saltabar", "100.0 KCAL", ""},
        // {"1", "saltabre", "100.0 KCAL", "10m"},
        {"7", "whatever", "[N/A]", ""},
        {"6", "Генерал Кеноби!", "100.0 KCAL", ""},
        {"2", "Какаобра", "99.0 KCAL", ""},
    }};

    ASSERT_EQ(expected_foods.size(), foods.size);
    ASSERT_NE(nullptr, foods.data);

    size_t i = 0;
    for (const auto& expected : expected_foods)
    {
        ASSERT_STREQ3(std::get<0>(expected), wfsv(foods.data[i].id));
        ASSERT_STREQ3(std::get<1>(expected), wfsv(foods.data[i].title));
        ASSERT_STREQ3(std::get<2>(expected), wfsv(foods.data[i].top_nutrient));
        ASSERT_STREQ3(
            std::get<3>(expected), wfsv(foods.data[i].preparation_time));
        i++;
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 1);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), expected_foods.size());
}

TEST_F(Test_wholth_pages_food, fetch_when_first_page_diff_locale)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food(&page, 8);

    auto wrap = PageWrap{page};

    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("2")));
    ASSERT_WHOLTH_OK(wholth_pages_fetch(page));

    const wholth_FoodArray foods = wholth_pages_food_array(page);

    // for (size_t i = 0; i < foods.size; i++)
    // {
    //     std::cout << "-------------\n"              //
    //               << wfsv(foods.data[i].id) << '\n' //
    //               << wfsv(foods.data[i].title) << '\n'
    //               << wfsv(foods.data[i].preparation_time) << '\n'
    //               << wfsv(foods.data[i].top_nutrient) << '\n';
    // }

    using sv = std::string_view;
    std::vector<std::tuple<sv, sv, sv, sv>> expected_foods{{
        {"6", "General Kenobi!", "100.0 KCAL", ""},
        {"5", "Hello there", "10.0 KCAL", ""},
        {"4", "Salia", "100.0 KCAL", ""},
        {"1", "Salta", "100.0 KCAL", "10m"},
        {"3", "Saltabar", "100.0 KCAL", ""},
        {"7", "whatever", "[N/A]", ""},
        {"2", "Какаобра", "99.0 KCAL", ""},
    }};

    ASSERT_EQ(expected_foods.size(), foods.size);
    ASSERT_NE(nullptr, foods.data);

    size_t i = 0;
    for (const auto& expected : expected_foods)
    {
        ASSERT_STREQ3(std::get<0>(expected), wfsv(foods.data[i].id));
        ASSERT_STREQ3(std::get<1>(expected), wfsv(foods.data[i].title));
        ASSERT_STREQ3(std::get<2>(expected), wfsv(foods.data[i].top_nutrient));
        ASSERT_STREQ3(
            std::get<3>(expected), wfsv(foods.data[i].preparation_time));
        i++;
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), expected_foods.size());
}

TEST_F(Test_wholth_pages_food, fetch_should_clamp_on_last_page)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food(&page, 8);
    auto wrap = PageWrap{page};

    ASSERT_WHOLTH_OK(err);

    ASSERT_TRUE(wholth_pages_skip_to(page, 99999));
    ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("1")));
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err);

    const auto foods = wholth_pages_food_array(page);

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), wholth_pages_max(page));
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), foods.size);
}

TEST_F(Test_wholth_pages_food, fetch_when_searched_by_title)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err, "#A");
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    ASSERT_WHOLTH_OK(wholth_pages_food_title(page, wtsv("Sal*")), "#B");
    ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("1")), "#C");
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err, "#D");

    const auto foods = wholth_pages_food_array(page);

    // for (size_t i = 0; i < foods.size; i++)
    // {
    //     std::cout << "-------------\n"              //
    //               << wfsv(foods.data[i].id) << '\n' //
    //               << wfsv(foods.data[i].title) << '\n'
    //               << wfsv(foods.data[i].preparation_time) << '\n'
    //               << wfsv(foods.data[i].top_nutrient) << '\n';
    // }

    using sv = std::string_view;
    std::vector<std::tuple<sv, sv, sv, sv>> expected_foods{{
        {"4", "Saliagr", "100.0 KCAL", ""},
        {"3", "Saltabar", "100.0 KCAL", ""},
        {"1", "saltabre", "100.0 KCAL", "10m"},
    }};

    ASSERT_EQ(expected_foods.size(), foods.size);
    ASSERT_NE(nullptr, foods.data);

    size_t i = 0;
    for (const auto& expected : expected_foods)
    {
        ASSERT_STREQ3(std::get<0>(expected), wfsv(foods.data[i].id));
        ASSERT_STREQ3(std::get<1>(expected), wfsv(foods.data[i].title));
        ASSERT_STREQ3(std::get<2>(expected), wfsv(foods.data[i].top_nutrient));
        ASSERT_STREQ3(
            std::get<3>(expected), wfsv(foods.data[i].preparation_time));
        i++;
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), foods.size);
}

TEST_F(Test_wholth_pages_food, fetch_when_searched_by_title_and_diff_locale)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food(&page, 8);
    auto wrap = PageWrap{page};
    ASSERT_WHOLTH_OK(err, "#A");
    ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    ASSERT_WHOLTH_OK(wholth_pages_food_title(page, wtsv("Sal*")), "#B");
    ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("2")), "#C");
    err = wholth_pages_fetch(page);

    ASSERT_WHOLTH_OK(err, "#D");

    const auto foods = wholth_pages_food_array(page);

    // for (size_t i = 0; i < foods.size; i++)
    // {
    //     std::cout << "-------------\n"              //
    //               << wfsv(foods.data[i].id) << '\n' //
    //               << wfsv(foods.data[i].title) << '\n'
    //               << wfsv(foods.data[i].preparation_time) << '\n'
    //               << wfsv(foods.data[i].top_nutrient) << '\n';
    // }

    using sv = std::string_view;
    std::vector<std::tuple<sv, sv, sv, sv>> expected_foods{{
        {"4", "Salia", "100.0 KCAL", ""},
        {"1", "Salta", "100.0 KCAL", "10m"},
        {"3", "Saltabar", "100.0 KCAL", ""},
    }};

    ASSERT_EQ(expected_foods.size(), foods.size);
    ASSERT_NE(nullptr, foods.data);

    size_t i = 0;
    for (const auto& expected : expected_foods)
    {
        ASSERT_STREQ3(std::get<0>(expected), wfsv(foods.data[i].id));
        ASSERT_STREQ3(std::get<1>(expected), wfsv(foods.data[i].title));
        ASSERT_STREQ3(std::get<2>(expected), wfsv(foods.data[i].top_nutrient));
        ASSERT_STREQ3(
            std::get<3>(expected), wfsv(foods.data[i].preparation_time));
        i++;
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), foods.size);
}

TEST_F(Test_wholth_pages_food, fetch_should_search_by_ingredients)
{
    auto& con = db::connection();

    //   5     6     2
    //   |     |     |
    //   3   4---7   1
    //   |
    //   2
    astmt(
        con,
        R"sql(
        INSERT OR REPLACE INTO recipe_step (id,recipe_id,seconds)
            VALUES
            (1,5,120),
            (2,3,20),
            (3,6,300),
            (4,2,10);
        INSERT OR REPLACE INTO recipe_step_food 
            (recipe_step_id, food_id, canonical_mass)
            VALUES
            (1,3,100),
            (2,2,200),
            (3,4,300),
            (3,7,400),
            (4,1,500)
        )sql");

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

    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food(&page, 8);
    auto wrap = PageWrap{};
    ASSERT_WHOLTH_OK(err, "#A");

    {
        ASSERT_TRUE(wholth_pages_skip_to(page, 0));
        ASSERT_WHOLTH_OK(wholth_pages_food_ingredients(
            page, wtsv("saliag*,Какао*")), "#B");
        ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("1")), "#C");
        err = wholth_pages_fetch(page);

        ASSERT_WHOLTH_OK(err, "#D");

        const auto foods = wholth_pages_food_array(page);
        // for (size_t i = 0; i < foods.size; i++)
        // {
        //     std::cout << "-------------\n"                //
        //               << wfsv(foods.data[i].id) << '\n' //
        //               << wfsv(foods.data[i].title) << '\n';
        // }
        using sv = std::string_view;

        std::vector<std::tuple<sv, sv, sv, sv>> expected_foods{{
            {"6", "Генерал Кеноби!", "100.0 KCAL", "5m"},
            {"5", "Hello there", "10.0 KCAL", "2m"},
            {"3", "Saltabar", "100.0 KCAL", "20s"},
        }};

        ASSERT_EQ(expected_foods.size(), foods.size);
        ASSERT_NE(nullptr, foods.data);

        size_t i = 0;
        for (const auto& expected : expected_foods)
        {
            ASSERT_STREQ3(std::get<0>(expected), wfsv(foods.data[i].id));
            ASSERT_STREQ3(std::get<1>(expected), wfsv(foods.data[i].title));
            ASSERT_STREQ3(
                std::get<2>(expected), wfsv(foods.data[i].top_nutrient));
            ASSERT_STREQ3(
                std::get<3>(expected), wfsv(foods.data[i].preparation_time));
            i++;
        }

        // ASSERT_GT(wholth_pages_max(page), 0);
        ASSERT_EQ(wholth_pages_current_page_num(page), 0);
        // ASSERT_GT(wholth_pages_count(page), 3);
        ASSERT_EQ(wholth_pages_span_size(page), 3);
    }

    // same as previos check but locale is different
    {
        ASSERT_TRUE(wholth_pages_skip_to(page, 0));
        ASSERT_WHOLTH_OK(wholth_pages_food_ingredients(
            page, wtsv("\"saliagr\",\"Какаобра\"")));
        ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("2")));
        err = wholth_pages_fetch(page);

        ASSERT_WHOLTH_OK(err);

        const auto foods = wholth_pages_food_array(page);
        // for (size_t i = 0; i < foods.size; i++)
        // {
        //     std::cout << "-------------\n"                //
        //               << wfsv(foods.data[i].id) << '\n' //
        //               << wfsv(foods.data[i].title) << '\n';
        // }
        using sv = std::string_view;

        std::vector<std::tuple<sv, sv, sv, sv>> expected_foods{{
            {"6", "General Kenobi!", "100.0 KCAL", "5m"},
            {"5", "Hello there", "10.0 KCAL", "2m"},
            {"3", "Saltabar", "100.0 KCAL", "20s"},
        }};

        ASSERT_EQ(expected_foods.size(), foods.size);
        ASSERT_NE(nullptr, foods.data);

        size_t i = 0;
        for (const auto& expected : expected_foods)
        {
            ASSERT_STREQ3(std::get<0>(expected), wfsv(foods.data[i].id));
            ASSERT_STREQ3(std::get<1>(expected), wfsv(foods.data[i].title));
            ASSERT_STREQ3(
                std::get<2>(expected), wfsv(foods.data[i].top_nutrient));
            ASSERT_STREQ3(
                std::get<3>(expected), wfsv(foods.data[i].preparation_time));
            i++;
        }

        // ASSERT_GT(wholth_pages_max(page), 0);
        ASSERT_EQ(wholth_pages_current_page_num(page), 0);
        // ASSERT_GT(wholth_pages_count(page), 3);
        ASSERT_EQ(wholth_pages_span_size(page), 3);
    }

    {
        ASSERT_TRUE(wholth_pages_skip_to(page, 0));
        ASSERT_WHOLTH_OK(wholth_pages_food_ingredients(page, wtsv("Salta")));
        ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("1")));
        err = wholth_pages_fetch(page);

        ASSERT_WHOLTH_OK(err);

        const auto foods = wholth_pages_food_array(page);

        // for (size_t i = 0; i < foods.size; i++)
        // {
        //     std::cout << "-------------\n"              //
        //               << wfsv(foods.data[i].id) << '\n' //
        //               << wfsv(foods.data[i].title) << '\n';
        // }

        ASSERT_GE(foods.size, 3);
        ASSERT_NE(nullptr, foods.data);

        ASSERT_STREQ2("5", wfsv(foods.data[0].id));
        ASSERT_STREQ2("3", wfsv(foods.data[1].id));
        ASSERT_STREQ2("2", wfsv(foods.data[2].id));

        // ASSERT_GT(wholth_pages_max(page), 0);
        // ASSERT_EQ(wholth_pages_current_page_num(page), 0);
        // ASSERT_GT(wholth_pages_count(page), 3);
        // ASSERT_EQ(wholth_pages_span_size(page), 8);
    }
}

TEST_F(Test_wholth_pages_food, when_by_id)
{
    // wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = nullptr;
    auto err = wholth_pages_food(&page, 8);

    auto wrap = PageWrap{page};

    ASSERT_WHOLTH_OK(err);
    ASSERT_WHOLTH_OK(wholth_pages_food_locale_id(page, wtsv("1")));
    ASSERT_WHOLTH_OK(wholth_pages_food_id(page, wtsv("1")));
    ASSERT_WHOLTH_OK(wholth_pages_fetch(page));

    const wholth_FoodArray foods = wholth_pages_food_array(page);

    // for (size_t i = 0; i < foods.size; i++)
    // {
    //     std::cout << "-------------\n"              //
    //               << wfsv(foods.data[i].id) << '\n' //
    //               << wfsv(foods.data[i].title) << '\n'
    //               << wfsv(foods.data[i].preparation_time) << '\n'
    //               << wfsv(foods.data[i].top_nutrient) << '\n';
    // }

    using sv = std::string_view;
    std::vector<std::tuple<sv, sv, sv, sv, sv>> expected_foods{{
        {"1", "saltabre", "100.0 KCAL", "10m", "61"},
    }};

    ASSERT_EQ(expected_foods.size(), foods.size);
    ASSERT_NE(nullptr, foods.data);

    size_t i = 0;
    for (const auto& expected : expected_foods)
    {
        ASSERT_STREQ3(std::get<0>(expected), wfsv(foods.data[i].id));
        ASSERT_STREQ3(std::get<1>(expected), wfsv(foods.data[i].title));
        ASSERT_STREQ3(std::get<2>(expected), wfsv(foods.data[i].top_nutrient));
        ASSERT_STREQ3(
            std::get<3>(expected), wfsv(foods.data[i].preparation_time));
        ASSERT_STREQ3(
            std::get<4>(expected), wfsv(foods.data[i].ingredients_mass_g));
        i++;
    }

    ASSERT_GE(wholth_pages_max(page), 0);
    ASSERT_EQ(wholth_pages_current_page_num(page), 0);
    ASSERT_GT(wholth_pages_count(page), 0);
    ASSERT_EQ(wholth_pages_span_size(page), expected_foods.size());
}
