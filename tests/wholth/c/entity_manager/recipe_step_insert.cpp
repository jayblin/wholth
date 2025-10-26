#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "utils/time_to_seconds.hpp"
#include "wholth/app_c.h"
#include "wholth/c/buffer.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/entity/recipe_step.h"
#include "wholth/c/entity_manager/recipe_step.h"
#include "wholth/c/forward.h"
#include "wholth/entity_manager/recipe_step.hpp"
#include "wholth/utils/is_valid_id.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <type_traits>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_em_recipe_step_insert : public ApplicationAwareTest
{
};

TEST_F(Test_wholth_em_recipe_step_insert, when_step_is_null)
{
    const std::string_view check_sql = R"sql(
    SELECT
        rsl.locale_id,
        rsl.description,
        rs.seconds
    FROM recipe_step_localisation rsl
    INNER JOIN recipe_step rs
        ON rsl.recipe_step_id = rs.id
    WHERE recipe_id = 10
    ORDER BY locale_id ASC
    )sql";
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_pre = ss.str();

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_Food food{.id = wtsv("10")};
    const auto err = wholth_em_recipe_step_insert(NULL, &food, scratch);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::recipe_step::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL, ec)
        << ec << ec.message();

    ss = {};
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_post = ss.str();
    ASSERT_STREQ3(check_data_pre, check_data_post);
}

TEST_F(Test_wholth_em_recipe_step_insert, when_time_is_null)
{
    const std::string_view check_sql = R"sql(
    SELECT
        rsl.locale_id,
        rsl.description,
        rs.seconds
    FROM recipe_step_localisation rsl
    INNER JOIN recipe_step rs
        ON rsl.recipe_step_id = rs.id
    WHERE recipe_id = 10
    ORDER BY locale_id ASC
    )sql";
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_pre = ss.str();

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_Food food{.id = wtsv("10")};

    wholth_RecipeStep step{
        // .time = wtsv("410s"),
        //.description = wtsv("case 4")
    };
    const auto err = wholth_em_recipe_step_insert(&step, &food, scratch);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::recipe_step::Code(err.code);
    ASSERT_EQ(
        wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL_TIME, ec)
        << ec << ec.message();

    ss = {};
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_post = ss.str();
    ASSERT_STREQ3(check_data_pre, check_data_post);
}

TEST_F(Test_wholth_em_recipe_step_insert, when_buffer_is_null)
{
    const std::string_view check_sql = R"sql(
    SELECT
        rsl.locale_id,
        rsl.description,
        rs.seconds
    FROM recipe_step_localisation rsl
    INNER JOIN recipe_step rs
        ON rsl.recipe_step_id = rs.id
    WHERE recipe_id = 10
    ORDER BY locale_id ASC
    )sql";
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_pre = ss.str();

    wholth_Food food{.id = wtsv("10")};

    wholth_RecipeStep step{
        .time = wtsv("410s"),
        .description = wtsv("case 4")
    };
    const auto err = wholth_em_recipe_step_insert(&step, &food, NULL);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    ss = {};
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_post = ss.str();
    ASSERT_STREQ3(check_data_pre, check_data_post);
}

TEST_F(Test_wholth_em_recipe_step_insert, when_time_is_invalid)
{
    const std::string_view check_sql = R"sql(
    SELECT
        rsl.locale_id,
        rsl.description,
        rs.seconds
    FROM recipe_step_localisation rsl
    INNER JOIN recipe_step rs
        ON rsl.recipe_step_id = rs.id
    WHERE recipe_id = 10
    ORDER BY locale_id ASC
    )sql";
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_pre = ss.str();

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_Food food{.id = wtsv("10")};

    std::vector<wholth_RecipeStep> cases{{
        {.time = wtsv(""), .description = wtsv("case 4")},
        {.time = wtsv("410b"), .description = wtsv("case 4")},
        {.time = wtsv("39"), .description = wtsv("case 4")},
        {.time = wtsv("-100s"), .description = wtsv("case 4")},
        {.time = wtsv("   "), .description = wtsv("case 4")},
    }};

    for (auto& step : cases)
    {
        std::string ident = fmt::format(" time: '{}'", wfsv(step.time));
        const auto err = wholth_em_recipe_step_insert(&step, &food, scratch);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message) << ident;
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message) << ident;

        std::error_code ec = utils::time_to_seconds_Code(err.code);
        ASSERT_TRUE(
            ec == utils::time_to_seconds_Code::INVALID_TIME_FORMAT ||
            ec == utils::time_to_seconds_Code::UNITLESS_VALUE ||
            ec == utils::time_to_seconds_Code::VALUE_IS_TOO_LARGE ||
            ec == utils::time_to_seconds_Code::EMPTY_VALUE)
            << ec << ec.message() << ident;

        ss = {};
        astmt(db::connection(), check_sql, [&](auto e) {
            ss << e.column_name << ":" << e.column_value << ';';
        });
        const std::string check_data_post = ss.str();
        ASSERT_STREQ3(check_data_pre, check_data_post) << ident;
    }
}

TEST_F(Test_wholth_em_recipe_step_insert, when_description_is_null)
{
    const std::string_view check_sql = R"sql(
    SELECT
        rsl.locale_id,
        rsl.description,
        rs.seconds
    FROM recipe_step_localisation rsl
    INNER JOIN recipe_step rs
        ON rsl.recipe_step_id = rs.id
    WHERE recipe_id = 10
    ORDER BY locale_id ASC
    )sql";
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_pre = ss.str();

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_Food food{.id = wtsv("10")};

    wholth_RecipeStep step{.time = wtsv("410s"),};
    ASSERT_EQ(NULL, step.description.data);
    const auto err = wholth_em_recipe_step_insert(&step, &food, scratch);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::recipe_step::Code(err.code);
    ASSERT_EQ(
        wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL_DESCRIPTION,
        ec)
        << ec << ec.message();

    ss = {};
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_post = ss.str();
    ASSERT_STREQ3(check_data_pre, check_data_post);
}

TEST_F(Test_wholth_em_recipe_step_insert, when_basic_case)
{
    std::string recipe_step_id = "";
    std::string recipe_id = "";
    astmt(
        db::connection(),
        "INSERT INTO food (created_at) VALUES ('10-10-2010') RETURNING id;",
        [&recipe_id](auto e) { recipe_id = e.column_value; });
    ASSERT_FALSE(recipe_id.empty());

    {
        wholth_user_locale_id(wtsv("1"));

        wholth_Food food{.id = wtsv(recipe_id)};
        wholth_RecipeStep step{
            .time = wtsv("44m"), .description = wtsv("do whatever!")};

        wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
        const auto err = wholth_em_recipe_step_insert(&step, &food, scratch);

        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec =
            wholth::entity_manager::recipe_step::Code(err.code);
        ASSERT_EQ(wholth::entity_manager::recipe_step::Code::OK, ec)
            << ec << ec.message();

        recipe_step_id = wfsv(step.id);
        ASSERT_TRUE(wholth::utils::is_valid_id(recipe_step_id)) << "value: '" << recipe_step_id << "'";

        const std::string check_sql = fmt::format(
            R"sql(
            SELECT
                rsl.locale_id,
                rsl.description,
                rs.seconds,
                rs.recipe_id
            FROM recipe_step_localisation rsl
            INNER JOIN recipe_step rs
                ON rsl.recipe_step_id = rs.id
            WHERE rs.id = {}
            ORDER BY locale_id ASC
            )sql",
            recipe_step_id);
        std::stringstream ss;
        astmt(db::connection(), check_sql, [&](auto e) {
            ss << e.column_name << ":" << e.column_value << ';';
        });
        const std::string check_data_pre = ss.str();
        ASSERT_STREQ3(
            fmt::format(
                "locale_id:1;description:do "
                "whatever!;seconds:2640;recipe_id:{};",
                recipe_id),
            check_data_pre);
    }

    // different locale
    {
        wholth_user_locale_id(wtsv("2"));

        wholth_Food food{.id = wtsv(recipe_id)};
        wholth_RecipeStep step{
            .time = wtsv("10m"),
            .description = wtsv("do whatever in locale 2!")};

        wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
        const auto err = wholth_em_recipe_step_insert(&step, &food, scratch);

        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec =
            wholth::entity_manager::recipe_step::Code(err.code);
        ASSERT_EQ(wholth::entity_manager::recipe_step::Code::OK, ec)
            << ec << ec.message();

        recipe_step_id = wfsv(step.id);
        ASSERT_TRUE(wholth::utils::is_valid_id(recipe_step_id)) << "value: '" << recipe_step_id << "'";

        const std::string check_sql = fmt::format(
            R"sql(
            SELECT
                rsl.locale_id,
                rsl.description,
                rs.seconds,
                rs.recipe_id
            FROM recipe_step_localisation rsl
            INNER JOIN recipe_step rs
                ON rsl.recipe_step_id = rs.id
            WHERE rs.id = {}
            ORDER BY locale_id ASC
            )sql",
            recipe_step_id);
        std::stringstream ss;
        astmt(db::connection(), check_sql, [&](auto e) {
            ss << e.column_name << ":" << e.column_value << ';';
        });
        const std::string check_data_pre = ss.str();
        ASSERT_STREQ3(
            fmt::format(
                "locale_id:1;description:do "
                "whatever!;seconds:2640;recipe_id:{0};"
                "locale_id:2;description:do "
                "whatever in locale 2!;seconds:2640;recipe_id:{0};",
                recipe_id),
            check_data_pre);
    }

    // different locale and duplicate
    {
        wholth_user_locale_id(wtsv("2"));

        wholth_Food food{.id = wtsv(recipe_id)};
        wholth_RecipeStep step{
            .time = wtsv("10m"),
            .description = wtsv("do whatever in locale 2!")};

        wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
        const auto err = wholth_em_recipe_step_insert(&step, &food, scratch);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec =
            wholth::entity_manager::recipe_step::Code(err.code);
        ASSERT_EQ(
            wholth::entity_manager::recipe_step::Code::
                RECIPE_STEP_ALREADY_EXISTS,
            ec)
            << ec << ec.message();

        const std::string_view cur_recipe_step_id = wfsv(step.id);
        ASSERT_FALSE(wholth::utils::is_valid_id(cur_recipe_step_id));

        const std::string check_sql = fmt::format(
            R"sql(
            SELECT
                rsl.locale_id,
                rsl.description,
                rs.seconds,
                rs.recipe_id
            FROM recipe_step_localisation rsl
            INNER JOIN recipe_step rs
                ON rsl.recipe_step_id = rs.id
            WHERE rs.id = {}
            ORDER BY locale_id ASC
            )sql",
            recipe_step_id);
        std::stringstream ss;
        astmt(db::connection(), check_sql, [&](auto e) {
            ss << e.column_name << ":" << e.column_value << ';';
        });
        const std::string check_data_pre = ss.str();
        ASSERT_STREQ3(
            fmt::format(
                "locale_id:1;description:do "
                "whatever!;seconds:2640;recipe_id:{0};"
                "locale_id:2;description:do "
                "whatever in locale 2!;seconds:2640;recipe_id:{0};",
                recipe_id),
            check_data_pre);
    }
}
