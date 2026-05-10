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

constexpr auto BIND_COUNT = 3;

#define PREPARE_COUNTERS(postfix)                                              \
    std::string count_exercise_localisation_fts5##postfix;                     \
    std::string count_exercise##postfix;

#define COUNT_ENTITIES(postfix)                                                \
    {                                                                          \
        auto& con = db::connection();                                          \
                                                                               \
        count_exercise_localisation_fts5##postfix = "bogus";                   \
        ASSERT_STMT_OK(                                                        \
            con,                                                               \
            "SELECT COUNT(rowid) FROM body_part_localisation_fts5",            \
            [&](auto e) {                                                      \
                count_exercise_localisation_fts5##postfix = e.column_value;    \
            });                                                                \
        ASSERT_NE("bogus", count_exercise_localisation_fts5##postfix);         \
                                                                               \
        count_exercise##postfix = "bogus";                                     \
        ASSERT_STMT_OK(con, "SELECT COUNT(id) FROM exercise", [&](auto e) {    \
            count_exercise##postfix = e.column_value;                          \
        });                                                                    \
        ASSERT_NE("bogus", count_exercise##postfix);                           \
    }

#define ASSERT_COUNTS_SAME(msg)                                                \
    ASSERT_STREQ3(                                                             \
        count_exercise_localisation_fts5_old,                                  \
        count_exercise_localisation_fts5_new)                                  \
        << msg;

#define ASSERT_COUNTS_NOT_SAME(msg)                                            \
    ASSERT_STRNE3(                                                             \
        count_exercise_localisation_fts5_old,                                  \
        count_exercise_localisation_fts5_new)                                  \
        << msg;                                                                \
    ASSERT_STRNE3(count_exercise_old, count_exercise_new) << msg;

#define ASSERT_EXERCISE_COUNT_EQ(expected, msg)                                \
    {                                                                          \
        std::string cnt{""};                                                   \
        ASSERT_STMT_OK(                                                        \
            db::connection(), "SELECT COUNT(id) FROM exercise", [&](auto e) {  \
                cnt = e.column_value;                                          \
            });                                                                \
        ASSERT_STREQ2(expected, cnt) << msg;                                   \
    }

class Test_wholth_sql_statements_exercise_insert : public ApplicationAwareTest
{
    void SetUp() override
    {
        ApplicationAwareTest::SetUp();
    }
};

TEST_F(Test_wholth_sql_statements_exercise_insert, should_report_errors)
{
    PREPARE_COUNTERS(_old);
    COUNT_ENTITIES(_old);

    struct _Case
    {
        wholth_exec_stmt_Bindable title;
        wholth_exec_stmt_Bindable description;
        wholth_exec_stmt_Bindable preferred_type_id;
        std::string_view          expected_stmt_err_msg;
    };
    std::vector<_Case> cases = {
        // 0
        {{wtsv("")},
         {wtsv("")},
         {wtsv("1")},
         {"Для упражнения нужно заполнить название!"}},
        // 1
        {{wtsv("aboba")},
         {wtsv("")},
         {wtsv("")},
         {"Идентификатор типа упраженения не валиден!"}},
        // 2
        {{wtsv("aboba")},
         {wtsv("")},
         {wtsv("-12")},
         {"Идентификатор типа упраженения не валиден!"}},
        // 3
        {{wtsv("aboba")},
         {wtsv("")},
         {wtsv("1a")},
         {"Идентификатор типа упраженения не валиден!"}},
        // 4
        {{wtsv("aboba")},
         {wtsv("")},
         {wtsv("999")},
         {"Не удалось найти тип упражнения с таким идентификатором!"}},
    };
    size_t i = 0;
    for (const auto& t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i++);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            t_case.title, t_case.description, t_case.preferred_type_id};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("exercise_insert.sql"),
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

        PREPARE_COUNTERS(_new);
        COUNT_ENTITIES(_new);
        ASSERT_COUNTS_SAME(msg);
        ASSERT_EXERCISE_COUNT_EQ("0", msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
    }
}

TEST_F(Test_wholth_sql_statements_exercise_insert, should_save_data)
{
    PREPARE_COUNTERS(_old);
    COUNT_ENTITIES(_old);

    struct _Case
    {
        wholth_exec_stmt_Bindable title;
        wholth_exec_stmt_Bindable description;
        wholth_exec_stmt_Bindable preferred_type_id;
        std::string_view          expected_data;
    };
    std::vector<_Case> cases = {
        // 0
        {
            {wtsv("aboba")},
            {wtsv("an aboba")},
            {wtsv("1")},
            "aboba,an aboba,1,",
        },
        // 1
        {
            {wtsv("aboba")},
            {wtsv("")},
            {wtsv("3")},
            "aboba,,3,",
        },
        // 2
        {
            {wtsv("aboba")},
            {wtsv("")},
            {nullptr, 0},
            "aboba,,,",
        }};
    size_t i = 0;
    for (const auto& t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i++);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            t_case.title, t_case.description, t_case.preferred_type_id};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("exercise_insert.sql"),
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
        ASSERT_NE(0, id.size) << msg;
        ASSERT_NE(nullptr, id.data) << msg;

        ASSERT_EXERCISE_COUNT_EQ("1", msg);

        std::string data{""};
        ASSERT_STMT_OK(
            db::connection(),
            fmt::format(
                "SELECT elf5.title, elf5.description, e.preferred_type_id "
                "FROM exercise e LEFT JOIN exercise_localisation_fts5 "
                "elf5 ON elf5.rowid = e.el_fts5_rowid WHERE e.id = {}",
                wfsv(id)),
            [&](auto e) { (data += e.column_value) += ","; });
        ASSERT_STREQ3(t_case.expected_data, data) << msg;

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
    }
}

TEST_F(Test_wholth_sql_statements_exercise_insert, when_duplicate_alias)
{
    PREPARE_COUNTERS(_old);
    COUNT_ENTITIES(_old);

    std::vector<wholth_exec_stmt_Bindable> cases = {
        // 0
        {wtsv("aboba")},
        // 1
        {wtsv("  aboba")},
        // 2
        {wtsv("aboba  ")},
        // 3
        {wtsv(" aboba  ")},
    };
    wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
        {wtsv("aboba")}, {}, {wtsv("1")}};
    wholth_exec_stmt_Args args = {
        .sql_file = wtsv("exercise_insert.sql"),
        .binds_size = BIND_COUNT,
        .binds = binds,
    };

    {
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK(err);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_OK2(err, wfsv(wholth_exec_stmt_Result_full_error_msg(res)));
    }

    size_t i = 0;
    for (const auto& t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i++);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});

        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        binds[0] = t_case;
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_NOK2(
            err, msg << wfsv(wholth_exec_stmt_Result_full_error_msg(res)));
        ASSERT_STREQ2(
            "Уже есть упражнение с таким названием!", wfsv(err.message))
            << msg << wfsv(wholth_exec_stmt_Result_full_error_msg(res));

        PREPARE_COUNTERS(_new);
        COUNT_ENTITIES(_new);
        ASSERT_COUNTS_SAME(msg);
        ASSERT_EXERCISE_COUNT_EQ("1", msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
    }
}
