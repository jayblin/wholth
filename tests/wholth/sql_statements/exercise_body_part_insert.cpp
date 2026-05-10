#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/error.h"
#include "wholth/c/exec_stmt.h"
#include "exec_stmt_helpers.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "assert.hpp"

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

constexpr auto BIND_COUNT = 2;

#define ASSERT_COUNT_EQ(expected, msg)                                         \
    {                                                                          \
        std::string cnt{""};                                                   \
        ASSERT_STMT_OK(                                                        \
            db::connection(),                                                  \
            "SELECT COUNT(exercise_id) FROM exercise_body_part",               \
            [&](auto e) { cnt = e.column_value; });                            \
        ASSERT_STREQ2(expected, cnt) << msg;                                   \
    }

class Test_wholth_sql_statements_exercise_body_part_insert
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
    }
};

TEST_F(
    Test_wholth_sql_statements_exercise_body_part_insert,
    should_report_errors)
{
    struct _Case
    {
        wholth_exec_stmt_Bindable exercise_id;
        wholth_exec_stmt_Bindable body_part_id;
        std::string_view          expected_stmt_err_msg;
    };
    std::vector<_Case> cases = {
        // 0
        {{wtsv("")}, {wtsv("1")}, {"Идентификатор упраженения не валиден!"}},
        // 1
        {{nullptr, 0}, {wtsv("1")}, {"Не передан идентификатор упражнения!"}},
        // 2
        {{wtsv("-1")}, {wtsv("1")}, {"Идентификатор упраженения не валиден!"}},
        // 3
        {{wtsv("abc")}, {wtsv("1")}, {"Идентификатор упраженения не валиден!"}},
        // 4
        {{wtsv("1")}, {wtsv("")}, {"Идентификатор части тела не валиден!"}},
        // 5
        {{wtsv("1")}, {nullptr, 0}, {"Не передан идентификатор части тела!"}},
        // 6
        {{wtsv("1")}, {wtsv("-1")}, {"Идентификатор части тела не валиден!"}},
        // 7
        {{wtsv("1")}, {wtsv("abc")}, {"Идентификатор части тела не валиден!"}},
    };
    size_t i = 0;
    for (const auto& t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i++);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            t_case.exercise_id, t_case.body_part_id};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("exercise_body_part_insert.sql"),
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

        ASSERT_COUNT_EQ("0", msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
    }
}

TEST_F(
    Test_wholth_sql_statements_exercise_body_part_insert,
    when_opimistic_case)
{
    const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
        {wtsv("1")}, {wtsv("2")}};
    wholth_exec_stmt_Args args = {
        .sql_file = wtsv("exercise_body_part_insert.sql"),
        .binds_size = BIND_COUNT,
        .binds = binds,
    };
    wholth_exec_stmt_Result* res = nullptr;
    auto                     err = wholth_exec_stmt_Result_new(&res);
    ASSERT_ERR_OK(err);
    err = wholth_exec_stmt(&args, res);
    ASSERT_ERR_OK2(err, wfsv(wholth_exec_stmt_Result_full_error_msg(res)));

    ASSERT_COUNT_EQ("1", "");

    std::string data{""};
    ASSERT_STMT_OK(
        db::connection(),
        "SELECT 'yes' FROM exercise_body_part WHERE exercise_id = 1 AND "
        "body_part_id = 2",
        [&](auto e) { (data += e.column_value) += ","; });
    ASSERT_STREQ2("yes,", data);
}

TEST_F(Test_wholth_sql_statements_exercise_body_part_insert, when_duplicate)
{
    const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
        {wtsv("1")}, {wtsv("2")}};
    wholth_exec_stmt_Args args = {
        .sql_file = wtsv("exercise_body_part_insert.sql"),
        .binds_size = BIND_COUNT,
        .binds = binds,
    };
    wholth_exec_stmt_Result* res = nullptr;
    auto                     err = wholth_exec_stmt_Result_new(&res);
    ASSERT_ERR_OK(err);
    err = wholth_exec_stmt(&args, res);
    ASSERT_ERR_OK2(err, wfsv(wholth_exec_stmt_Result_full_error_msg(res)));

    err = wholth_exec_stmt(&args, res);
    ASSERT_ERR_NOK2(err, wfsv(wholth_exec_stmt_Result_full_error_msg(res)));
    ASSERT_STREQ2("Упражнение уже привязано к части тела!", wfsv(err.message))
        << wfsv(wholth_exec_stmt_Result_full_error_msg(res));

    ASSERT_COUNT_EQ("1", "");

    std::string data{""};
    ASSERT_STMT_OK(
        db::connection(),
        "SELECT 'yes' FROM exercise_body_part WHERE exercise_id = 1 AND "
        "body_part_id = 2",
        [&](auto e) { (data += e.column_value) += ","; });
    ASSERT_STREQ2("yes,", data);
}
