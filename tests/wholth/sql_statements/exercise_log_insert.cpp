#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/error.h"
#include "wholth/c/exec_stmt.h"
#include "exec_stmt_helpers.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>
#include "assert.hpp"

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

constexpr auto BIND_COUNT = 4;

#define ASSERT_EXERCISE_LOG_COUNT_EQ(expected, msg)                            \
    {                                                                          \
        std::string cnt{""};                                                   \
        ASSERT_STMT_OK(                                                        \
            db::connection(),                                                  \
            "SELECT COUNT(exercise_id) FROM exercise_log",                     \
            [&](auto e) { cnt = e.column_value; });                            \
        ASSERT_STREQ2(expected, cnt) << msg;                                   \
    }

class Test_wholth_sql_statements_exercise_log_insert
    : public ApplicationAwareTest
{
    void SetUp() override
    {
        ApplicationAwareTest::SetUp();
    }
};

TEST_F(Test_wholth_sql_statements_exercise_log_insert, should_report_errors)
{
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
        ASSERT_ERR_OK2(err, wfsv(wholth_exec_stmt_Result_full_error_msg(res)));
    }

    struct _Case
    {
        wholth_exec_stmt_Bindable exercise_id;
        wholth_exec_stmt_Bindable type_id;
        wholth_exec_stmt_Bindable value;
        wholth_exec_stmt_Bindable created_at;
        std::string_view          expected_stmt_err_msg;
    };
    std::vector<_Case> cases = {
        // 0
        {{}, {}, {}, {}, {"Не передан идентификатор упражнения!"}},
        // 1
        {{nullptr, 0}, {}, {}, {}, {"Не передан идентификатор упражнения!"}},
        // 2
        {{wtsv("-1")}, {}, {}, {}, {"Идентификатор упраженения не валиден!"}},
        // 3
        {{wtsv("4b")}, {}, {}, {}, {"Идентификатор упраженения не валиден!"}},
        // 4
        {{wtsv("44")},
         {},
         {},
         {},
         {"Не удалось найти упражнение с таким идентификатором!"}},
        // 5
        {{wtsv("1")},
         {wtsv("-10")},
         {},
         {},
         {"Идентификатор типа упраженения не валиден!"}},
        // 6
        {{wtsv("1")},
         {wtsv("1v")},
         {},
         {},
         {"Идентификатор типа упраженения не валиден!"}},
        // 7
        {{wtsv("1")},
         {wtsv("1")},
         {wtsv("-10")},
         {},
         {"Значение не может быть меньше 0!"}},
        // 8
        {{wtsv("1")},
         {wtsv("1")},
         {wtsv("10")},
         {wtsv("aboba")},
         {"Невалидная дата/время!"}},
    };
    size_t i = 0;
    for (const auto& t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i++);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            t_case.exercise_id,
            t_case.type_id,
            t_case.value,
            t_case.created_at};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("exercise_log_insert.sql"),
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

        ASSERT_EXERCISE_LOG_COUNT_EQ("0", msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
    }
}
TEST_F(Test_wholth_sql_statements_exercise_log_insert, should_save_data)
{
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
        ASSERT_ERR_OK2(err, wfsv(wholth_exec_stmt_Result_full_error_msg(res)));
    }

    struct _Case
    {
        wholth_exec_stmt_Bindable type_id;
        wholth_exec_stmt_Bindable value;
        wholth_exec_stmt_Bindable created_at;
        std::string_view          expected_data;
        std::string_view          expected_date;
    };
    std::vector<_Case> cases = {
        // 0
        {{wtsv("2")}, {wtsv("10")}, {}, "1,2,10,", ""},
        // 1
        {{wtsv("2")}, {wtsv("40b")}, {}, "1,2,40,", ""},
        // 2
        {{wtsv("3")}, {wtsv("19.3")}, {}, "1,3,19,", ""},
        // 3
        {{wtsv("3")},
         {wtsv("19.3")},
         {wtsv("2022-03-03 11:25")},
         "1,3,19,",
         "2022-03-03 11:25,"},
    };
    size_t i = 0;
    for (const auto& t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i++);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            {wtsv("1")}, t_case.type_id, t_case.value, t_case.created_at};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("exercise_log_insert.sql"),
            .binds_size = BIND_COUNT,
            .binds = binds,
        };
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_OK2(
            err, msg << wfsv(wholth_exec_stmt_Result_full_error_msg(res)));

        const auto id = wholth_exec_stmt_Result_at(res, 0, 0);
        ASSERT_NE(nullptr, id.data) << msg;
        ASSERT_NE(0, id.size) << msg;

        ASSERT_EXERCISE_LOG_COUNT_EQ("1", msg);
        std::stringstream data{};
        ASSERT_STMT_OK(
            db::connection(),
            fmt::format(
                "SELECT exercise_id,type_id,value FROM exercise_log WHERE "
                "exercise_id={}",
                wfsv(id)),
            [&](auto e) { data << e.column_value << ","; });
        ASSERT_STREQ3(t_case.expected_data, data.str()) << msg;

        data = {};
        ASSERT_STMT_OK(
            db::connection(),
            fmt::format(
                "SELECT created_at FROM exercise_log WHERE "
                "exercise_id={}",
                wfsv(id)),
            [&](auto e) { data << e.column_value << ","; });
        const auto date = data.str();
        ASSERT_STRNEQ2("", date) << msg;

        if (t_case.expected_date != "")
        {
            ASSERT_STREQ3(t_case.expected_date, date) << msg;
        }

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
    }
}
