#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "utils/time_to_seconds.hpp"
#include "wholth/app_c.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/entity/recipe_step.h"
#include "wholth/c/entity_manager/recipe_step.h"
#include "wholth/c/forward.h"
#include "wholth/entity_manager/recipe_step.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <type_traits>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_em_recipe_step_update : public ApplicationAwareTest
{
};

TEST_F(Test_wholth_em_recipe_step_update, when_step_is_null)
{
    astmt(
        db::connection(),
        "INSERT INTO food (id, created_at) VALUES (555550, 'now');"
        "INSERT INTO recipe_step (id, recipe_id, priority, seconds) VALUES "
        " (888880, 20, 0, 50);"
        "INSERT INTO recipe_step_localisation (recipe_step_id, locale_id, "
        "description) "
        " VALUES (888880, 1, 'do thing!')");

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_recipe_step_update(NULL, scratch);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::recipe_step::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::recipe_step::Code::RECIPE_STEP_NULL, ec)
        << ec << ec.message();

    const std::string_view check_sql = R"sql(
    SELECT
        rsl.locale_id,
        rsl.description,
        rs.seconds
    FROM recipe_step_localisation rsl
    INNER JOIN recipe_step rs
        ON rsl.recipe_step_id = rs.id
    WHERE recipe_step_id = 888880
    ORDER BY locale_id ASC
    )sql";
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_post = ss.str();
    ASSERT_STREQ2(
        "locale_id:1;description:do thing!;seconds:50;", check_data_post);
}

TEST_F(Test_wholth_em_recipe_step_update, when_time_is_null)
{
    astmt(
        db::connection(),
        "INSERT INTO food (id, created_at) VALUES (555550, 'now');"
        "INSERT INTO recipe_step (id, recipe_id, priority, seconds) VALUES "
        " (888880, 20, 0, 50);"
        "INSERT INTO recipe_step_localisation (recipe_step_id, locale_id, "
        "description) "
        " VALUES (888880, 1, 'do thing!')");

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_RecipeStep step = wholth_entity_recipe_step_init();
    step.id = wtsv("888880");
    step.description = wtsv("other thing!");
    ASSERT_EQ(NULL, step.time.data);

    wholth_app_locale_id(wtsv("1"));

    const auto err = wholth_em_recipe_step_update(&step, scratch);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::recipe_step::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::recipe_step::Code::OK, ec)
        << ec << ec.message();

    const std::string_view check_sql = R"sql(
    SELECT
        rsl.locale_id,
        rsl.description,
        rs.seconds
    FROM recipe_step_localisation rsl
    INNER JOIN recipe_step rs
        ON rsl.recipe_step_id = rs.id
    WHERE recipe_step_id = 888880
    ORDER BY locale_id ASC
    )sql";
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_post = ss.str();
    ASSERT_STREQ2(
        "locale_id:1;description:other thing!;seconds:50;", check_data_post);
}

TEST_F(Test_wholth_em_recipe_step_update, when_description_is_null)
{
    astmt(
        db::connection(),
        "INSERT INTO food (id, created_at) VALUES (555550, 'now');"
        "INSERT INTO recipe_step (id, recipe_id, priority, seconds) VALUES "
        " (888880, 20, 0, 50);"
        "INSERT INTO recipe_step_localisation (recipe_step_id, locale_id, "
        "description) "
        " VALUES (888880, 1, 'do thing!')");

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_RecipeStep step = wholth_entity_recipe_step_init();
    step.id = wtsv("888880");
    step.time = wtsv("1m");
    ASSERT_EQ(NULL, step.description.data);

    wholth_app_locale_id(wtsv("1"));

    const auto err = wholth_em_recipe_step_update(&step, scratch);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::recipe_step::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::recipe_step::Code::OK, ec)
        << ec << ec.message();

    const std::string_view check_sql = R"sql(
    SELECT
        rsl.locale_id,
        rsl.description,
        rs.seconds
    FROM recipe_step_localisation rsl
    INNER JOIN recipe_step rs
        ON rsl.recipe_step_id = rs.id
    WHERE recipe_step_id = 888880
    ORDER BY locale_id ASC
    )sql";
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_post = ss.str();
    ASSERT_STREQ2(
        "locale_id:1;description:do thing!;seconds:60;", check_data_post);
}

TEST_F(Test_wholth_em_recipe_step_update, when_buffer_is_null)
{
    astmt(
        db::connection(),
        "INSERT INTO food (id, created_at) VALUES (555550, 'now');"
        "INSERT INTO recipe_step (id, recipe_id, priority, seconds) VALUES "
        " (888880, 20, 0, 50);"
        "INSERT INTO recipe_step_localisation (recipe_step_id, locale_id, "
        "description) "
        " VALUES (888880, 1, 'do thing!')");

    wholth_RecipeStep step = wholth_entity_recipe_step_init();
    const auto err = wholth_em_recipe_step_update(&step, NULL);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::recipe_step::Code(err.code);
    ASSERT_NE(wholth::entity_manager::recipe_step::Code::OK, ec)
        << ec << ec.message();

    const std::string_view check_sql = R"sql(
    SELECT
        rsl.locale_id,
        rsl.description,
        rs.seconds
    FROM recipe_step_localisation rsl
    INNER JOIN recipe_step rs
        ON rsl.recipe_step_id = rs.id
    WHERE recipe_step_id = 888880
    ORDER BY locale_id ASC
    )sql";
    std::stringstream ss;
    astmt(db::connection(), check_sql, [&](auto e) {
        ss << e.column_name << ":" << e.column_value << ';';
    });
    const std::string check_data_post = ss.str();
    ASSERT_STREQ2(
        "locale_id:1;description:do thing!;seconds:50;", check_data_post);
}

TEST_F(Test_wholth_em_recipe_step_update, when_time_is_invalid)
{
    astmt(
        db::connection(),
        "INSERT INTO food (id, created_at) VALUES (555550, 'now');"
        "INSERT INTO recipe_step (id, recipe_id, priority, seconds) VALUES "
        " (888880, 20, 0, 50);"
        "INSERT INTO recipe_step_localisation (recipe_step_id, locale_id, "
        "description) "
        " VALUES (888880, 1, 'do thing!')");

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();
    wholth_Food food{.id = wtsv("10")};

    auto id_sv = wtsv("888880");
    auto description_sv = wtsv("case 4");
    std::vector<wholth_StringView> cases{{
        wtsv(""),
        wtsv("-100s"),
        wtsv("39b"),
        wtsv("  "),
    }};

    for (auto& time : cases)
    {
        wholth_RecipeStep step = {
            .id = id_sv,
            .time = time,
            .description = description_sv,
        };
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

        const std::string_view check_sql = R"sql(
        SELECT
            rsl.locale_id,
            rsl.description,
            rs.seconds
        FROM recipe_step_localisation rsl
        INNER JOIN recipe_step rs
            ON rsl.recipe_step_id = rs.id
        WHERE recipe_step_id = 888880
        ORDER BY locale_id ASC
        )sql";
        std::stringstream ss;
        astmt(db::connection(), check_sql, [&](auto e) {
            ss << e.column_name << ":" << e.column_value << ';';
        });
        const std::string check_data_post = ss.str();
        ASSERT_STREQ2(
            "locale_id:1;description:do thing!;seconds:50;", check_data_post);
    }
}

TEST_F(Test_wholth_em_recipe_step_update, when_basic_case)
{
    astmt(
        db::connection(),
        "INSERT INTO food (id, created_at) VALUES (555550, 'now');"
        "INSERT INTO recipe_step (id, recipe_id, priority, seconds) VALUES "
        " (888880, 20, 0, 50);"
        "INSERT INTO recipe_step_localisation (recipe_step_id, locale_id, "
        "description) "
        " VALUES (888880, 1, 'do thing!')");

    wholth_Buffer* scratch = wholth_buffer_ring_pool_element();

    {
        wholth_RecipeStep step = wholth_entity_recipe_step_init();
        step.id = wtsv("888880");
        step.time = wtsv("15m");
        step.description = wtsv("another thing!");

        wholth_app_locale_id(wtsv("1"));

        const auto err = wholth_em_recipe_step_update(&step, scratch);

        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec =
            wholth::entity_manager::recipe_step::Code(err.code);
        ASSERT_EQ(wholth::entity_manager::recipe_step::Code::OK, ec)
            << ec << ec.message();

        const std::string_view check_sql = R"sql(
        SELECT
            rsl.locale_id,
            rsl.description,
            rs.seconds
        FROM recipe_step_localisation rsl
        INNER JOIN recipe_step rs
            ON rsl.recipe_step_id = rs.id
        WHERE recipe_step_id = 888880
        ORDER BY locale_id ASC
        )sql";
        std::stringstream ss;
        astmt(db::connection(), check_sql, [&](auto e) {
            ss << e.column_name << ":" << e.column_value << ';';
        });
        const std::string check_data_post = ss.str();
        ASSERT_STREQ2(
            "locale_id:1;description:another thing!;seconds:900;",
            check_data_post);
    }

    // another locale
    {
        wholth_RecipeStep step = wholth_entity_recipe_step_init();
        step.id = wtsv("888880");
        step.time = wtsv("10m");
        step.description = wtsv("another locale thing!");

        wholth_app_locale_id(wtsv("2"));

        const auto err = wholth_em_recipe_step_update(&step, scratch);

        ASSERT_EQ(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::error_code ec =
            wholth::entity_manager::recipe_step::Code(err.code);
        ASSERT_EQ(wholth::entity_manager::recipe_step::Code::OK, ec)
            << ec << ec.message();

        const std::string_view check_sql = R"sql(
        SELECT
            rsl.locale_id,
            rsl.description,
            rs.seconds
        FROM recipe_step_localisation rsl
        INNER JOIN recipe_step rs
            ON rsl.recipe_step_id = rs.id
        WHERE recipe_step_id = 888880
        ORDER BY locale_id ASC
        )sql";
        std::stringstream ss;
        astmt(db::connection(), check_sql, [&](auto e) {
            ss << e.column_name << ":" << e.column_value << ';';
        });
        const std::string check_data_post = ss.str();
        ASSERT_STREQ2(
            "locale_id:1;description:another thing!;seconds:600;"
            "locale_id:2;description:another locale thing!;seconds:600;",
            check_data_post);
    }
}
