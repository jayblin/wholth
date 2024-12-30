#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <system_error>
#include <tuple>
#include <vector>
#include "db/db.hpp"
#include "helpers.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/cmake_vars.h"

typedef std::tuple<
    std::string_view,      // case_name
    db::status::Condition, // condition to compare error_code to
    std::error_code,       // error_code to compare condtion to
    bool                   // whether to check euqality or inequality
    >
    test_case_t;

class ErrorCategoryTest : public testing::TestWithParam<test_case_t>
{
  protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_P(ErrorCategoryTest, equivalence)
{
    const auto [_, condition, ec, flag] = GetParam();

    if (flag)
    {
        ASSERT_TRUE(condition == ec);
    }
    else
    {
        ASSERT_TRUE(condition != ec);
    }
}

INSTANTIATE_TEST_SUITE_P(
    Instantiantion,
    ErrorCategoryTest,
    testing::Values(
        test_case_t{
            "A",
            db::status::Condition::OK,
            std::error_code{SQLITE_OK, std::generic_category()},
            false},
        test_case_t{
            "B",
            db::status::Condition::OK,
            sqlw::status::Code{SQLITE_OK},
            true},
        test_case_t{
            "C",
            db::status::Condition::OK,
            sqlw::status::Code{SQLITE_DONE},
            true},
        test_case_t{
            "D",
            db::status::Condition::OK,
            sqlw::status::Code{SQLITE_ROW},
            true},
        test_case_t{
            "E",
            db::status::Condition::OK,
            sqlw::status::Code{SQLITE_ERROR},
            false},
        test_case_t{"F", db::status::Condition::OK, db::status::Code::OK, true},
        test_case_t{
            "G",
            db::status::Condition::OK,
            db::status::Code::MIGRATION_TABLE_DOES_NOT_EXIST,
            false},
        test_case_t{
            "H",
            db::status::Condition::OK,
            db::status::Code::MIGRATION_FAILED,
            false},
        test_case_t{
            "I",
            db::status::Condition::OK,
            db::status::Code::MIGRATION_LOG_FAILED,
            false}),
    [](const testing::TestParamInfo<ErrorCategoryTest::ParamType>& info) {
        return std::string{std::get<0>(info.param)};
    });

TEST_F(InMemoryDatabaseAwareTest, make_migrations_table)
{
    std::error_code ec = db::migration::make_migration_table(&db_con);
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;

    std::stringstream ss;
    ec = sqlw::Statement{&db_con}(
        "SELECT name FROM sqlite_schema WHERE type='table'",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ","; });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2("name:migration,", ss.str());
}

TEST_F(InMemoryDatabaseAwareTest, migrate_case_A)
{
    // 1. migrations table exists and has no executed migrations. good connection;
    // 1.1 old ones are not executed;
    // 1.2 no error is returned;
    // 1.3 returns vector of filenames of executed migrations;

    std::error_code ec;
    ec = db::migration::make_migration_table(&db_con);
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;

    std::filesystem::path mock_migrations_dir {PROJECT_DIR};
    mock_migrations_dir /= "tests/db/mock_migrations";

    const auto list = db::migration::list_sorted_migrations(mock_migrations_dir);

    const db::migration::MigrateResult result = db::migration::migrate({
        .con = &db_con,
        .migrations = list,
        .log = true,
    });
    ASSERT_TRUE(db::status::Condition::OK == result.error_code) << result.error_code;

    std::stringstream ss;
    ec = sqlw::Statement{&db_con}(
        "SELECT * FROM user; SELECT * from user2",
        [&] (auto e) {
            ss << e.column_name << ":" << e.column_value << ";";
        }
    );
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2(
        "id:1;name:super_user;id:2;name:normal_user;id:1;name:user21;id:2;name:user22;",
        ss.str()
    );

    ASSERT_EQ(3, result.executed_migrations.size());
    ASSERT_EQ("001-a.sql", result.executed_migrations[0]);
    ASSERT_EQ("001-b.sql", result.executed_migrations[1]);
    ASSERT_EQ("002-a.sql", result.executed_migrations[2]);

    ss.str("");
    ec = sqlw::Statement{&db_con}(
        "SELECT filename FROM migration ORDER BY executed_at ASC",
        [&] (auto e) {
            ss << e.column_value << ",";
        }
    );
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2(
        "001-a.sql,001-b.sql,002-a.sql,",
        ss.str()
    );
}

TEST_F(InMemoryDatabaseAwareTest, migrate_case_B)
{
    // 2. if has executed migrations.
    // 2.1 old ones are not executed;
    // 2.2 error is returned;
    // 2.3 returns almost empty vector of filenames of executed migrations;

    std::error_code ec;
    ec = db::migration::make_migration_table(&db_con);
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;

    std::filesystem::path mock_migrations_dir {PROJECT_DIR};
    mock_migrations_dir /= "tests/db/mock_migrations";

    ec = sqlw::Statement{&db_con}("INSERT INTO migration (filename,executed_at) VALUES ('001-a.sql', 0), ('001-b.sql', 1)");
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;

    const auto list = db::migration::list_sorted_migrations(mock_migrations_dir);

    const db::migration::MigrateResult result = db::migration::migrate({
        .con = &db_con,
        .migrations = list,
        .log = true,
    });
    ASSERT_TRUE(db::status::Condition::OK == result.error_code) << result.error_code;

    std::stringstream ss;
    ec = sqlw::Statement{&db_con}(
        "SELECT * from user2",
        [&] (auto e) {
            ss << e.column_name << ":" << e.column_value << ";";
        }
    );
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2(
        "id:1;name:user21;id:2;name:user22;",
        ss.str()
    );

    ASSERT_EQ(1, result.executed_migrations.size());
    ASSERT_EQ("002-a.sql", result.executed_migrations[0]);

    ss.str("");
    ec = sqlw::Statement{&db_con}(
        "SELECT filename FROM migration ORDER BY executed_at ASC",
        [&] (auto e) {
            ss << e.column_value << ",";
        }
    );
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2(
        "001-a.sql,001-b.sql,002-a.sql,",
        ss.str()
    );
}

TEST_F(InMemoryDatabaseAwareTest, migrate_case_C)
{
    // 3. good connection. migrations table does not exist;
    // 3.1 migrations table is created;
    // 3.2 migrations are executed;
    // 3.3 migrations are loged in the table;
    // 3.4 no error is returned;
    // 3.5 returns vector of filenames of executed migrations;

    std::error_code ec;
    std::filesystem::path mock_migrations_dir {PROJECT_DIR};
    mock_migrations_dir /= "tests/db/mock_migrations";

    const auto list = db::migration::list_sorted_migrations(mock_migrations_dir);

    const db::migration::MigrateResult result = db::migration::migrate({
        .con = &db_con,
        .migrations = list,
        .log = true,
    });

    ASSERT_FALSE(db::status::Condition::OK == result.error_code) << result.error_code;
    ASSERT_TRUE(db::status::Code::MIGRATION_TABLE_DOES_NOT_EXIST == result.error_code) << result.error_code;
    ASSERT_EQ(0, result.executed_migrations.size());

    std::stringstream ss;
    ec = sqlw::Statement{&db_con}(
        "SELECT filename FROM migration ORDER BY executed_at ASC",
        [&] (auto e) {
            ss << e.column_value << ",";
        }
    );
    ASSERT_TRUE(db::status::Condition::OK != ec) << ec;
    ASSERT_STREQ2(
        "",
        ss.str()
    );
}

TEST_F(InMemoryDatabaseAwareTest, migrate_case_D)
{
    // 5. good connection. migrations table does exist. some of migrations contain
    // an error;
    // 5.1 migrations are executed, except the one that has an error and
    // the ones past it;
    // 5.2 migrations are logged in the table, except the one that has an error;
    // 5.2 error is returned with a filename of erroneous migration;
    // 5.3 returns vector of filenames of executed migrations. which must not
    // contain migrations past the one that failed;

    std::error_code ec;
    ec = db::migration::make_migration_table(&db_con);
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;

    std::filesystem::path mock_migrations_dir {PROJECT_DIR};
    mock_migrations_dir /= "tests/db/mock_erroneous_migrations";

    const auto list = db::migration::list_sorted_migrations(mock_migrations_dir);

    const db::migration::MigrateResult result = db::migration::migrate({
        .con = &db_con,
        .migrations = list,
        .log = true,
    });
    ASSERT_TRUE(db::status::Condition::OK != result.error_code) << result.error_code;
    ASSERT_EQ(db::status::Code::MIGRATION_FAILED, result.error_code) << ec;

    std::stringstream ss;
    ec = sqlw::Statement{&db_con}(
        "SELECT name FROM sqlite_schema WHERE type='table' AND name LIKE 'user%'",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ","; });
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2("name:user,", ss.str());

    ss.str("");
    ec = sqlw::Statement{&db_con}(
        "SELECT * from user",
        [&] (auto e) {
            ss << e.column_name << ":" << e.column_value << ";";
        }
    );
    ASSERT_TRUE(db::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2("", ss.str());

    ASSERT_EQ(1, result.executed_migrations.size());
    ASSERT_EQ("001-a.sql", result.executed_migrations[0]);
    ASSERT_STREQ2("001-b.sql", result.problematic_migration);
}
