#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/error.h"
#include "wholth/c/exec_stmt.h"
#include "exec_stmt_helpers.hpp"
#include <array>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>
#include "assert.hpp"

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

constexpr auto BIND_COUNT = 1;
constexpr auto ENTRIES_COUNT = 3;

#define ASSERT_EXERCISE_LOG_COUNT_EQ(expected, msg)                            \
    {                                                                          \
        std::string cnt{""};                                                   \
        ASSERT_STMT_OK(                                                        \
            db::connection(),                                                  \
            "SELECT COUNT(exercise_id) FROM exercise_log",                     \
            [&](auto e) { cnt = e.column_value; });                            \
        ASSERT_STREQ2(expected, cnt) << msg;                                   \
    }

class Test_wholth_sql_statements_exercise_log_delete
    : public ApplicationAwareTest
{
    void SetUp() override
    {
        ApplicationAwareTest::SetUp();

        {
            const wholth_exec_stmt_Bindable binds[3] = {
                {wtsv("aboba")}, {wtsv("an aboba")}, {wtsv("1")}};
            wholth_exec_stmt_Args args = {
                .sql_file = wtsv("exercise_insert.sql"),
                .binds_size = 3,
                .binds = binds,
            };
            wholth_exec_stmt_Result* res = nullptr;
            auto                     err = wholth_exec_stmt_Result_new(&res);
            ASSERT_ERR_OK(err);
            err = wholth_exec_stmt(&args, res);
            ASSERT_ERR_OK2(
                err, wfsv(wholth_exec_stmt_Result_full_error_msg(res)));
        }

        for (auto i = 0; i < ENTRIES_COUNT; i++)
        {
            const wholth_exec_stmt_Bindable binds[4] = {
                {wtsv("1")}, {wtsv("1")}, {wtsv("10")}, {}};
            wholth_exec_stmt_Args args = {
                .sql_file = wtsv("exercise_log_insert.sql"),
                .binds_size = 4,
                .binds = binds,
            };
            wholth_exec_stmt_Result* res = nullptr;
            auto                     err = wholth_exec_stmt_Result_new(&res);
            ASSERT_ERR_OK(err);
            err = wholth_exec_stmt(&args, res);
            ASSERT_ERR_OK2(
                err, wfsv(wholth_exec_stmt_Result_full_error_msg(res)));

            this->log_ids[i] = wfsv(wholth_exec_stmt_Result_at(res, 0, 0));
        }
    }

  protected:
    std::array<std::string, ENTRIES_COUNT> log_ids{};
};

TEST_F(Test_wholth_sql_statements_exercise_log_delete, should_report_errors)
{
    struct _Case
    {
        wholth_exec_stmt_Bindable id;
        std::string_view          expected_stmt_err_msg;
    };
    std::vector<_Case> cases = {
        // 0
        {{wtsv("-1")}, {"Идентификатор лога не валиден!"}},
        // 1
        {{wtsv("1d")}, {"Идентификатор лога не валиден!"}},
    };
    size_t i = 0;
    for (const auto& t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i++);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {t_case.id};
        wholth_exec_stmt_Args           args = {
                      .sql_file = wtsv("exercise_log_delete.sql"),
                      .binds_size = BIND_COUNT,
                      .binds = binds,
        };
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_NOK2(
            err, msg << wfsv(wholth_exec_stmt_Result_full_error_msg(res)));
        ASSERT_STREQ3(t_case.expected_stmt_err_msg, wfsv(err.message))
            << msg << wfsv(wholth_exec_stmt_Result_full_error_msg(res));

        ASSERT_EXERCISE_LOG_COUNT_EQ("3", msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
    }
}

TEST_F(Test_wholth_sql_statements_exercise_log_delete, should_noop)
{
    struct _Case
    {
        wholth_exec_stmt_Bindable id;
    };
    std::vector<_Case> cases = {
        // 0
        {{wtsv("33")}},
        // 1
        {{}},
        // 2
        {{nullptr, 0}},
    };
    size_t i = 0;
    for (const auto& t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i++);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {t_case.id};
        wholth_exec_stmt_Args           args = {
                      .sql_file = wtsv("exercise_log_delete.sql"),
                      .binds_size = BIND_COUNT,
                      .binds = binds,
        };
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_OK2(
            err, msg << wfsv(wholth_exec_stmt_Result_full_error_msg(res)));

        ASSERT_EXERCISE_LOG_COUNT_EQ("3", msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
    }
}

TEST_F(Test_wholth_sql_statements_exercise_log_delete, should_delete)
{
    ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
    const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
        {wtsv(this->log_ids[0])}};
    wholth_exec_stmt_Args args = {
        .sql_file = wtsv("exercise_log_delete.sql"),
        .binds_size = BIND_COUNT,
        .binds = binds,
    };
    wholth_exec_stmt_Result* res = nullptr;
    auto                     err = wholth_exec_stmt_Result_new(&res);
    ASSERT_ERR_OK(err);
    err = wholth_exec_stmt(&args, res);
    ASSERT_ERR_OK2(err, wfsv(wholth_exec_stmt_Result_full_error_msg(res)));

    ASSERT_EXERCISE_LOG_COUNT_EQ("2", "");

    for (auto i = 1; i < ENTRIES_COUNT; i++)
    {
        std::stringstream data{""};
        ASSERT_STMT_OK(
            db::connection(),
            fmt::format(
                "SELECT id FROM exercise_log WHERE id={}",
                this->log_ids[i]),
            [&](auto e) { data << e.column_value << ","; });
        ASSERT_STRNEQ2("", data.str()) << "idx #" << i;
    }
}
