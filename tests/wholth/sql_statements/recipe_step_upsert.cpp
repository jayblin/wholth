#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/exec_stmt.h"
#include "exec_stmt_helpers.hpp"
#include <string>
#include <vector>
#include "assert.hpp"

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_sql_statements_recipe_step_upsert
    : public ApplicationAwareTest
{
    void SetUp() override
    {
        ApplicationAwareTest::SetUp();

        ASSERT_STMT_OK(
            db::connection(),
            "INSERT OR REPLACE INTO recipe_step "
            " (id, recipe_id, seconds) VALUES "
            " (1, 10, 120),"
            " (2, 20, 60)",
            [](auto) {});

        ASSERT_STMT_OK(
            db::connection(),
            "INSERT OR REPLACE INTO recipe_step_localisation "
            " (recipe_step_id, locale_id, description) VALUES "
            " (1, 1, 'description 1 of 1'),"
            " (1, 2, 'description 2 of 1'),"
            " (2, 1, 'description 1 of 2')",
            [](auto) {});
    }
};

#define ASSERT_OTHER_RECIPE_STEP_UNCHANGED_1()                                 \
    {                                                                          \
        std::string result{""};                                                \
        auto&       con = db::connection();                                    \
        ASSERT_STMT_OK(                                                        \
            con,                                                               \
            "SELECT recipe_id,seconds,description FROM recipe_step INNER "     \
            "JOIN "                                                            \
            "recipe_step_localisation ON recipe_step_id = id WHERE id = 1 "    \
            "ORDER BY locale_id ASC",                                          \
            [&](auto e) { (result += e.column_value) += ";"; });               \
        ASSERT_STREQ2(                                                         \
            "10;120;description 1 of 1;10;120;description 2 of 1;", result);   \
    }

#define ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2()                                 \
    {                                                                          \
        std::string result{""};                                                \
        auto&       con = db::connection();                                    \
        ASSERT_STMT_OK(                                                        \
            con,                                                               \
            "SELECT recipe_id,seconds,description FROM recipe_step INNER "     \
            "JOIN "                                                            \
            "recipe_step_localisation ON recipe_step_id = id WHERE id = 2 "    \
            "ORDER BY id ASC, locale_id ASC",                                  \
            [&](auto e) { (result += e.column_value) += ";"; });               \
        ASSERT_STREQ2("20;60;description 1 of 2;", result);                    \
    }

static void count_initial_entities(
    std::string& count_rs,
    std::string& count_rsl)
{
    auto& con = db::connection();

    count_rs = "bogus";
    ASSERT_STMT_OK(con, "SELECT COUNT(id) FROM recipe_step", [&](auto e) {
        count_rs = e.column_value;
    });
    ASSERT_NE("bogus", count_rs);

    count_rsl = "bogus";
    ASSERT_STMT_OK(
        con, "SELECT COUNT(*) FROM recipe_step_localisation", [&](auto e) {
            count_rsl = e.column_value;
        });
    ASSERT_NE("bogus", count_rsl);
}

TEST_F(Test_wholth_sql_statements_recipe_step_upsert, when_args_is_nullptr)
{
    std::string count_rs_old;
    std::string count_rsl_old;
    count_initial_entities(count_rs_old, count_rsl_old);

    const auto err = wholth_exec_stmt(nullptr);
    ASSERT_ERR_NOK(err);

    std::string count_rs_new;
    std::string count_rsl_new;
    count_initial_entities(count_rs_new, count_rsl_new);
    ASSERT_STREQ3(count_rs_old, count_rs_new);
    ASSERT_STREQ3(count_rsl_old, count_rsl_new);

    ASSERT_OTHER_RECIPE_STEP_UNCHANGED_1();
    ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2();
}

TEST_F(Test_wholth_sql_statements_recipe_step_upsert, when_bad_food_id)
{
    std::string count_rs_old;
    std::string count_rsl_old;
    count_initial_entities(count_rs_old, count_rsl_old);

    std::vector<wholth_exec_stmt_Bindable> ids = {
        {},
        {wtsv("")},
        {wtsv("abc")},
        {wtsv("-123")},
        {wtsv("1f")},
        {wtsv("  102")},
    };
    for (const auto id : ids)
    {
        const wholth_exec_stmt_Bindable binds[5] = {id, {}, {}, {}, {}};
        wholth_exec_stmt_Args           args = {
                      .sql_file = wtsv("recipe_step_upsert.sql"),
                      .binds_size = 5,
                      .binds = binds,
        };
        const auto err = wholth_exec_stmt(&args);
        ASSERT_ERR_NOK(err);
        ASSERT_ERR_MSG(err, "Невалидный идентифкатор рецепта(пищи)!");

        std::string count_rs_new;
        std::string count_rsl_new;
        count_initial_entities(count_rs_new, count_rsl_new);
        ASSERT_STREQ3(count_rs_old, count_rs_new);
        ASSERT_STREQ3(count_rsl_old, count_rsl_new);

        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_1();
        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2();
    }
}

// TEST_F(Test_wholth_sql_statements_recipe_step_upsert, when_bad_seconds)
// {
//     std::string count_rs_old;
//     std::string count_rsl_old;
//     count_initial_entities(count_rs_old, count_rsl_old);
//
//     std::vector<wholth_exec_stmt_Bindable> seconds = {
//         {},
//         {wtsv("")},
//         {wtsv("abc")},
//         {wtsv("-123")},
//         {wtsv("1f")},
//         {wtsv("  102")},
//     };
//     for (const auto second : seconds)
//     {
//         wholth_Buffer*                  buf =
//         wholth_buffer_ring_pool_element(); const wholth_exec_stmt_Bindable
//         binds[4] = {
//             {wtsv("1")}, second, {}, {}};
//         wholth_exec_stmt_Args args = {
//             .sql_file = wtsv("recipe_step_upsert.sql"),
//             .binds_size = 4,
//             .binds = binds,
//             .buffer = buf,
//         };
//         const auto err = wholth_exec_stmt(&args);
//         ASSERT_ERR_NOK(err);
//         ASSERT_ERR_MSG(err, "Невалидный идентифкатор локали!");
//
//         std::string count_rs_new;
//         std::string count_rsl_new;
//         count_initial_entities(count_rs_new, count_rsl_new);
//         ASSERT_STREQ3(count_rs_old, count_rs_new);
//         ASSERT_STREQ3(count_rsl_old, count_rsl_new);
//     }
// }

TEST_F(Test_wholth_sql_statements_recipe_step_upsert, when_bad_locale_id)
{
    std::string count_rs_old;
    std::string count_rsl_old;
    count_initial_entities(count_rs_old, count_rsl_old);

    std::vector<wholth_exec_stmt_Bindable> locale_ids = {
        {},
        {wtsv("")},
        {wtsv("abc")},
        {wtsv("-123")},
        {wtsv("1f")},
        {wtsv("  102")},
    };
    for (const auto locale_id : locale_ids)
    {
        const wholth_exec_stmt_Bindable binds[5] = {
            {wtsv("1")}, {wtsv("630")}, locale_id, {}, {}};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("recipe_step_upsert.sql"),
            .binds_size = 5,
            .binds = binds,
        };
        const auto err = wholth_exec_stmt(&args);
        ASSERT_ERR_NOK(err);
        ASSERT_ERR_MSG(err, "Невалидный идентифкатор локали!");

        std::string count_rs_new;
        std::string count_rsl_new;
        count_initial_entities(count_rs_new, count_rsl_new);
        ASSERT_STREQ3(count_rs_old, count_rs_new);
        ASSERT_STREQ3(count_rsl_old, count_rsl_new);

        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_1();
        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2();
    }
}

TEST_F(Test_wholth_sql_statements_recipe_step_upsert, when_good_insert)
{
    std::string count_rs_old;
    std::string count_rsl_old;
    count_initial_entities(count_rs_old, count_rsl_old);

    const wholth_exec_stmt_Bindable binds[5] = {
        {wtsv("1")},
        {wtsv("630")},
        {wtsv("1")},
        {wtsv("2")},
        {wtsv("a description")}};
    wholth_exec_stmt_Args args = {
        .sql_file = wtsv("recipe_step_upsert.sql"),
        .binds_size = 5,
        .binds = binds,
    };
    wholth::exec_stmt::ResultWrap res{};
    const auto                    err = wholth_exec_stmt(&args, res.handle);
    ASSERT_ERR_OK(err);

    std::string count_rs_new;
    std::string count_rsl_new;
    count_initial_entities(count_rs_new, count_rsl_new);
    ASSERT_STRNE3(count_rs_old, count_rs_new);
    ASSERT_STRNE3(count_rsl_old, count_rsl_new);

    const auto res_id = wfsv(wholth_exec_stmt_Result_at(res.handle));
    ASSERT_STRNEQ2("", res_id);

    {
        std::string acc = "";
        ASSERT_STMT_OK(
            db::connection(),
            fmt::format(
                "SELECT id,recipe_id,seconds FROM recipe_step WHERE id = {0}",
                res_id),
            [&](auto e) { (acc += e.column_value) += ";"; });
        ASSERT_NE("", acc);
        ASSERT_STREQ3(fmt::format("{0};1;630;", res_id), acc);
    }

    {
        std::string acc = "";
        ASSERT_STMT_OK(
            db::connection(),
            fmt::format(
                "SELECT description FROM recipe_step_localisation WHERE "
                "recipe_step_id = {0} AND locale_id = 2",
                res_id),
            [&](auto e) { acc += e.column_value; });
        ASSERT_STREQ2("a description", acc);
    }

    ASSERT_OTHER_RECIPE_STEP_UNCHANGED_1();
    ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2();
}

TEST_F(
    Test_wholth_sql_statements_recipe_step_upsert,
    insert_with_null_description)
{
    std::string count_rs_old;
    std::string count_rsl_old;
    count_initial_entities(count_rs_old, count_rsl_old);

    std::vector<wholth_StringView> descriptions{{
        {},                          //
        {.data = nullptr, .size = 0} //
    }};
    for (auto description : descriptions)
    {
        const wholth_exec_stmt_Bindable binds[5] = {
            {wtsv("1")},
            {wtsv("630")},
            {wtsv("1")},
            {wtsv("2")},
            {description}};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("recipe_step_upsert.sql"),
            .binds_size = 5,
            .binds = binds,
        };
        wholth::exec_stmt::ResultWrap res{};
        const auto                    err = wholth_exec_stmt(&args, res.handle);
        ASSERT_ERR_OK(err);

        std::string count_rs_new;
        std::string count_rsl_new;
        count_initial_entities(count_rs_new, count_rsl_new);
        ASSERT_STRNE3(count_rs_old, count_rs_new);
        ASSERT_STRNE3(count_rsl_old, count_rsl_new);

        const auto res_id = wfsv(wholth_exec_stmt_Result_at(res.handle));
        ASSERT_STRNEQ2("", res_id);

        {
            std::string acc = "";
            ASSERT_STMT_OK(
                db::connection(),
                fmt::format(
                    "SELECT id,recipe_id,seconds FROM recipe_step WHERE id = "
                    "{0}",
                    res_id),
                [&](auto e) { (acc += e.column_value) += ";"; });
            ASSERT_NE("", acc);
            ASSERT_STREQ3(fmt::format("{0};1;630;", res_id), acc);
        }

        {
            std::string acc = "";
            ASSERT_STMT_OK(
                db::connection(),
                fmt::format(
                    "SELECT description FROM recipe_step_localisation WHERE "
                    "recipe_step_id = {0} AND locale_id = 2",
                    res_id),
                [&](auto e) { acc += e.column_value; });
            ASSERT_STREQ2("", acc);
        }

        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_1();
        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2();
    }
}

TEST_F(
    Test_wholth_sql_statements_recipe_step_upsert,
    insert_with_empty_description)
{
    std::string count_rs_old;
    std::string count_rsl_old;
    count_initial_entities(count_rs_old, count_rsl_old);

    std::vector<wholth_StringView> descriptions{{
        {},                          //
        {.data = nullptr, .size = 0} //
    }};
    for (auto description : descriptions)
    {
        const wholth_exec_stmt_Bindable binds[5] = {
            {wtsv("1")},
            {wtsv("630")},
            {wtsv("1")},
            {wtsv("1")},
            {description}};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("recipe_step_upsert.sql"),
            .binds_size = 5,
            .binds = binds,
        };
        wholth::exec_stmt::ResultWrap res{};
        const auto                    err = wholth_exec_stmt(&args, res.handle);
        ASSERT_ERR_OK(err);

        std::string count_rs_new;
        std::string count_rsl_new;
        count_initial_entities(count_rs_new, count_rsl_new);
        ASSERT_STRNE3(count_rs_old, count_rs_new);
        ASSERT_STRNE3(count_rsl_old, count_rsl_new);

        const auto res_id = wfsv(wholth_exec_stmt_Result_at(res.handle));
        ASSERT_STRNEQ2("", res_id);

        {
            std::string acc = "";
            ASSERT_STMT_OK(
                db::connection(),
                fmt::format(
                    "SELECT id,recipe_id,seconds FROM recipe_step WHERE id = "
                    "{0}",
                    res_id),
                [&](auto e) { (acc += e.column_value) += ";"; });
            ASSERT_NE("", acc);
            ASSERT_STREQ3(fmt::format("{0};1;630;", res_id), acc);
        }

        {
            std::string acc = "";
            ASSERT_STMT_OK(
                db::connection(),
                fmt::format(
                    "SELECT description FROM recipe_step_localisation WHERE "
                    "recipe_step_id = {0} AND locale_id = 2",
                    res_id),
                [&](auto e) { acc += e.column_value; });
            ASSERT_STREQ2("", acc);
        }

        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_1();
        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2();
    }
}

TEST_F(Test_wholth_sql_statements_recipe_step_upsert, when_good_update)
{
    std::string count_rs_old;
    std::string count_rsl_old;
    count_initial_entities(count_rs_old, count_rsl_old);

    const wholth_exec_stmt_Bindable binds[5] = {
        {wtsv("10")},        // recipe_id
        {wtsv("630")},       // seconds
        {wtsv("10")},        // recipe_id
        {wtsv("2")},         // locale_id
        {wtsv("redescribe")} // description
    };
    wholth_exec_stmt_Args args = {
        .sql_file = wtsv("recipe_step_upsert.sql"),
        .binds_size = 5,
        .binds = binds,
    };
    wholth::exec_stmt::ResultWrap res{};
    const auto                    err = wholth_exec_stmt(&args, res.handle);
    ASSERT_ERR_OK(err);

    std::string count_rs_new;
    std::string count_rsl_new;
    count_initial_entities(count_rs_new, count_rsl_new);
    ASSERT_STREQ3(count_rs_old, count_rs_new);
    ASSERT_STREQ3(count_rsl_old, count_rsl_new);

    const auto res_id = wfsv(wholth_exec_stmt_Result_at(res.handle));
    ASSERT_STREQ2("1", res_id);

    std::string acc = "";
    ASSERT_STMT_OK(
        db::connection(),
        fmt::format(
            "SELECT id,seconds,locale_id,description FROM recipe_step LEFT "
            "JOIN "
            "recipe_step_localisation ON recipe_step_id = id WHERE id = {0} "
            "ORDER BY locale_id ASC",
            res_id),
        [&](auto e) { (acc += e.column_value) += ";"; });
    ASSERT_NE("", acc);

    ASSERT_STREQ2("1;630;1;description 1 of 1;1;630;2;redescribe;", acc);
    ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2();
}

TEST_F(
    Test_wholth_sql_statements_recipe_step_upsert,
    when_update_with_null_description)
{
    std::string count_rs_old;
    std::string count_rsl_old;
    count_initial_entities(count_rs_old, count_rsl_old);

    std::vector<wholth_StringView> descriptions{{
        {},                          //
        {.data = nullptr, .size = 0} //
    }};
    for (auto description : descriptions)
    {
        const wholth_exec_stmt_Bindable binds[5] = {
            {wtsv("10")},  // recipe_id
            {wtsv("630")}, // seconds
            {wtsv("10")},  // recipe_id
            {wtsv("2")},   // locale_id
            {description}};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("recipe_step_upsert.sql"),
            .binds_size = 5,
            .binds = binds,
        };
        wholth::exec_stmt::ResultWrap res{};
        const auto                    err = wholth_exec_stmt(&args, res.handle);
        ASSERT_ERR_OK(err);

        std::string count_rs_new;
        std::string count_rsl_new;
        count_initial_entities(count_rs_new, count_rsl_new);
        ASSERT_STREQ3(count_rs_old, count_rs_new);
        ASSERT_STREQ3(count_rsl_old, count_rsl_new);

        const auto res_id = wfsv(wholth_exec_stmt_Result_at(res.handle));
        ASSERT_STREQ2("1", res_id);

        std::string acc = "";
        ASSERT_STMT_OK(
            db::connection(),
            fmt::format(
                "SELECT id,seconds,locale_id,description FROM recipe_step LEFT "
                "JOIN "
                "recipe_step_localisation ON recipe_step_id = id WHERE id = "
                "{0} "
                "ORDER BY locale_id ASC",
                res_id),
            [&](auto e) { (acc += e.column_value) += ";"; });
        ASSERT_NE("", acc);

        ASSERT_STREQ2(
            "1;630;1;description 1 of 1;1;630;2;description 2 of 1;", acc);
        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2();
    }
}

TEST_F(
    Test_wholth_sql_statements_recipe_step_upsert,
    when_update_with_empty_description)
{
    std::string count_rs_old;
    std::string count_rsl_old;
    count_initial_entities(count_rs_old, count_rsl_old);

    std::vector<wholth_StringView> descriptions{{
        {.data = "", .size = 0} //
    }};
    for (auto description : descriptions)
    {
        const wholth_exec_stmt_Bindable binds[5] = {
            {wtsv("10")},  // recipe_id
            {wtsv("630")}, // seconds
            {wtsv("10")},  // recipe_id
            {wtsv("1")},   // locale_id
            {description}};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("recipe_step_upsert.sql"),
            .binds_size = 5,
            .binds = binds,
        };
        wholth::exec_stmt::ResultWrap res{};
        const auto                    err = wholth_exec_stmt(&args, res.handle);
        ASSERT_ERR_OK(err);

        std::string count_rs_new;
        std::string count_rsl_new;
        count_initial_entities(count_rs_new, count_rsl_new);
        ASSERT_STREQ3(count_rs_old, count_rs_new);
        ASSERT_STREQ3(count_rsl_old, count_rsl_new);

        const auto res_id = wfsv(wholth_exec_stmt_Result_at(res.handle));
        ASSERT_STREQ2("1", res_id);

        std::string acc = "";
        ASSERT_STMT_OK(
            db::connection(),
            fmt::format(
                "SELECT id,seconds,locale_id,description FROM recipe_step LEFT "
                "JOIN "
                "recipe_step_localisation ON recipe_step_id = id WHERE id = "
                "{0} "
                "ORDER BY locale_id ASC",
                res_id),
            [&](auto e) { (acc += e.column_value) += ";"; });
        ASSERT_NE("", acc);

        ASSERT_STREQ2(
            "1;630;1;;1;630;2;description 2 of 1;", acc);
        ASSERT_OTHER_RECIPE_STEP_UNCHANGED_2();
    }
}
