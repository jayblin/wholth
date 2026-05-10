#include "db/db.hpp"
#include "fmt/core.h"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/exec_stmt.h"
#include "exec_stmt_helpers.hpp"
#include <string>
#include <vector>
#include "assert.hpp"

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

constexpr auto BIND_COUNT = 3;

#define ASSERT_TREE(tree, msg)                                                 \
    {                                                                          \
        std::string result{""};                                                \
        auto&       con = db::connection();                                    \
        ASSERT_STMT_OK(                                                        \
            con,                                                               \
            "select "                                                          \
            "bp.id, bpn.lft, bpn.rgt "                                         \
            "from body_part bp "                                               \
            "left join body_part_nset bpn "                                    \
            "on bpn.body_part_id = bp.id "                                     \
            "order by lft asc ",                                               \
            [&](auto e) { (result += e.column_value) += ","; });               \
        ASSERT_STREQ3(tree, result) << msg;                                    \
    }

#define ASSERT_LEAF_DATA(leaf_id, data, msg)                                   \
    {                                                                          \
        std::string result{""};                                                \
        auto&       con = db::connection();                                    \
        ASSERT_STMT_OK(                                                        \
            con,                                                               \
            fmt::format(                                                       \
                "select "                                                      \
                "bpl.title, bpl.description "                                  \
                "from body_part bp "                                           \
                "left join body_part_nset bpn "                                \
                "on bpn.body_part_id = bp.id "                                 \
                "left join body_part_localisation_fts5 bpl "                   \
                "on bpl.rowid = bp.bpl_fts5_rowid "                            \
                "where bp.id = {} "                                            \
                "order by lft asc ",                                           \
                leaf_id),                                                      \
            [&](auto e) { (result += e.column_value) += ","; });               \
        ASSERT_STREQ3(data, result) << msg;                                    \
    }

class Test_wholth_sql_statements_body_part_insert : public ApplicationAwareTest
{
    void SetUp() override
    {
        ApplicationAwareTest::SetUp();
        //
        // ASSERT: tree is of this shape
        //
        //                      тело(1)
        //                      1|12
        //  ---------------------------------------------------
        // 2|3     4|5         6|7             8|9          10|11
        // шея(2)  руки(3)  спина верх(4)   спина низ(5)    ноги(6)
        //
        ASSERT_TREE(
            std::string_view{"1,1,12,"
                             "2,2,3,"
                             "3,4,5,"
                             "4,6,7,"
                             "5,8,9,"
                             "6,10,11,"},
            " - precondition_1");
    }
};

static void count_initial_entities(
    std::string& count_bp,
    std::string& count_bpl,
    std::string& count_bpns)
{
    auto& con = db::connection();

    count_bp = "bogus";
    ASSERT_STMT_OK(
        con,
        "SELECT COUNT(rowid) FROM body_part_localisation_fts5",
        [&](auto e) { count_bp = e.column_value; });
    ASSERT_NE("bogus", count_bp);

    count_bpl = "bogus";
    ASSERT_STMT_OK(con, "SELECT COUNT(id) FROM body_part", [&](auto e) {
        count_bpl = e.column_value;
    });
    ASSERT_NE("bogus", count_bpl);

    count_bpns = "bogus";
    ASSERT_STMT_OK(
        con, "SELECT COUNT(body_part_id) FROM body_part_nset", [&](auto e) {
            count_bpns = e.column_value;
        });
    ASSERT_NE("bogus", count_bpns);
}

TEST_F(
    Test_wholth_sql_statements_body_part_insert,
    should_save_localisation_data)
{
    std::string count_bp_old;
    std::string count_bpl_old;
    std::string count_bpns_old;
    count_initial_entities(count_bp_old, count_bpl_old, count_bpns_old);

    struct _Case
    {
        wholth_exec_stmt_Bindable title;
        wholth_exec_stmt_Bindable description;
        std::string_view          expected_data;
    };
    std::vector<_Case> cases = {
        {{wtsv("ARM")}, {}, {"arm,,"}},
        {{wtsv("Leg")}, {wtsv("an ARM")}, {"leg,an ARM,"}},
        {{wtsv("  forearm   ")}, {wtsv(" an arm ")}, {"forearm, an arm ,"}},
    };
    size_t i = 0;
    for (const auto t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            t_case.title, t_case.description, {wtsv("1")}};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("body_part_insert.sql"),
            .binds_size = BIND_COUNT,
            .binds = binds,
        };
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_OK2(err, msg);
        // ASSERT_ERR_MSG(err, "Невалидный идентифкатор рецепта(пищи)!");

        std::string count_bp_new;
        std::string count_bpl_new;
        std::string count_bpns_new;
        count_initial_entities(count_bp_new, count_bpl_new, count_bpns_new);
        ASSERT_STRNE3(count_bp_old, count_bp_new) << msg;
        ASSERT_STRNE3(count_bpl_old, count_bpl_new) << msg;
        ASSERT_STRNE3(count_bpns_old, count_bpns_new) << msg;
        const auto result_id = wholth_exec_stmt_Result_at(res, 0, 0);
        ASSERT_NE(nullptr, result_id.data) << msg;
        ASSERT_NE(0, result_id.size) << msg;

        //                      тело(1)
        //                      1|14
        //  ---------------------------------------------------------------
        // 2|3     4|5         6|7             8|9          10|11       12|13
        // шея(2)  руки(3)  спина верх(4)   спина низ(5)    ноги(6)      (7)
        ASSERT_TREE(
            std::string_view{"1,1,14,"
                             "2,2,3,"
                             "3,4,5,"
                             "4,6,7,"
                             "5,8,9,"
                             "6,10,11,"
                             "7,12,13,"},
            msg);

        ASSERT_LEAF_DATA(wfsv(result_id), t_case.expected_data, msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
        i++;
    }
}

TEST_F(Test_wholth_sql_statements_body_part_insert, when_bad_title)
{
    std::string count_bp_old;
    std::string count_bpl_old;
    std::string count_bpns_old;
    count_initial_entities(count_bp_old, count_bpl_old, count_bpns_old);

    struct _Case
    {
        wholth_exec_stmt_Bindable title;
        wholth_exec_stmt_Bindable description;
    };
    std::vector<_Case> cases = {
        {
            {},
            {},
        },
        {
            {},
            {nullptr, 0},
        },
        {
            {},
            {wtsv("")},
        },
        {
            {nullptr, 0},
            {},
        },
        {
            {wtsv("")},
            {},
        },
    };
    size_t i = 0;
    for (const auto t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i);
        // astmt(db::connection(), "SAVEPOINT _savepoint_");
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            t_case.title, t_case.description, {wtsv("1")}};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("body_part_insert.sql"),
            .binds_size = BIND_COUNT,
            .binds = binds,
        };
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_NOK(err);
        ASSERT_STREQ2("Недопустимое название части тела!", wfsv(err.message));

        std::string count_bp_new;
        std::string count_bpl_new;
        std::string count_bpns_new;
        count_initial_entities(count_bp_new, count_bpl_new, count_bpns_new);
        ASSERT_STREQ3(count_bp_old, count_bp_new) << msg;
        ASSERT_STREQ3(count_bpl_old, count_bpl_new) << msg;
        ASSERT_STREQ3(count_bpns_old, count_bpns_new) << msg;

        ASSERT_TREE(
            std::string_view{"1,1,12,"
                             "2,2,3,"
                             "3,4,5,"
                             "4,6,7,"
                             "5,8,9,"
                             "6,10,11,"},
            msg);

        // astmt(db::connection(), "ROLLBACK TO _savepoint_");
        i++;
    }
}

TEST_F(Test_wholth_sql_statements_body_part_insert, when_bad_parent_id)
{
    std::string count_bp_old;
    std::string count_bpl_old;
    std::string count_bpns_old;
    count_initial_entities(count_bp_old, count_bpl_old, count_bpns_old);

    std::vector<wholth_exec_stmt_Bindable> parent_ids = {
        {wtsv("-1")},
        {wtsv("idk")},
        {wtsv("1idk")},
        {wtsv("1idk")},
        {wtsv("999")},
        {wtsv("")},
    };
    size_t i = 0;
    for (const auto parent_id : parent_ids)
    {
        std::string msg = fmt::format(" - case #{} - ", i);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            {wtsv("arm")}, {wtsv("an arm")}, parent_id};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("body_part_insert.sql"),
            .binds_size = BIND_COUNT,
            .binds = binds,
        };
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_NOK2(err, msg);
        ASSERT_STREQ2(
            "Невалидный идентификатор ролительского элемента!",
            wfsv(err.message));

        std::string count_bp_new;
        std::string count_bpl_new;
        std::string count_bpns_new;
        count_initial_entities(count_bp_new, count_bpl_new, count_bpns_new);
        ASSERT_STREQ3(count_bp_old, count_bp_new) << msg;
        ASSERT_STREQ3(count_bpl_old, count_bpl_new) << msg;
        ASSERT_STREQ3(count_bpns_old, count_bpns_new) << msg;
        const auto result_id = wholth_exec_stmt_Result_at(res, 0, 0);
        ASSERT_EQ(0, result_id.size) << msg;

        ASSERT_TREE(
            std::string_view{"1,1,12,"
                             "2,2,3,"
                             "3,4,5,"
                             "4,6,7,"
                             "5,8,9,"
                             "6,10,11,"},
            msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
        i++;
    }
}

TEST_F(Test_wholth_sql_statements_body_part_insert, when_null_parent_id)
{
    std::string count_bp_old;
    std::string count_bpl_old;
    std::string count_bpns_old;
    count_initial_entities(count_bp_old, count_bpl_old, count_bpns_old);

    std::vector<wholth_exec_stmt_Bindable> parent_ids = {
        {},
        {{nullptr, 0}},
    };
    size_t i = 0;
    for (const auto parent_id : parent_ids)
    {
        std::string msg = fmt::format(" - case #{} - ", i);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            {wtsv("arm")}, {wtsv("an arm")}, parent_id};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("body_part_insert.sql"),
            .binds_size = BIND_COUNT,
            .binds = binds,
        };
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_OK2(err, msg);

        std::string count_bp_new;
        std::string count_bpl_new;
        std::string count_bpns_new;
        count_initial_entities(count_bp_new, count_bpl_new, count_bpns_new);
        ASSERT_STRNE3(count_bp_old, count_bp_new) << msg;
        ASSERT_STRNE3(count_bpl_old, count_bpl_new) << msg;
        ASSERT_STRNE3(count_bpns_old, count_bpns_new) << msg;
        const auto result_id = wholth_exec_stmt_Result_at(res, 0, 0);
        ASSERT_NE(0, result_id.size) << msg;

        ASSERT_TREE(
            fmt::format(
                "1,1,14,"
                "2,2,3,"
                "3,4,5,"
                "4,6,7,"
                "5,8,9,"
                "6,10,11,"
                "{},12,13,",
                wfsv(result_id)),
            msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
        i++;
    }
}

TEST_F(
    Test_wholth_sql_statements_body_part_insert,
    should_insert_leaves_correctly)
{
    struct _Case
    {
        wholth_exec_stmt_Bindable parent_id;
        std::string_view          expected_tree;
    };
    std::vector<_Case> cases = {
        //                      тело(1)
        //                      1|14
        //                       |
        //  -----------------------------------------------------
        // 2|3     4|5         6|9             10|11          12|13
        // шея(2)  руки(3)  спина верх(4)   спина низ(5)    ноги(6)
        //                      |
        //                     7|8
        //                    arm(7)
        {{wtsv("4")},
         {"1,1,14,"
          "2,2,3,"
          "3,4,5,"
          "4,6,9,"
          "7,7,8,"
          "5,10,11,"
          "6,12,13,"}},
        //                      тело(1)
        //                      1|16
        //                       |
        //  ------------------------------------------------------
        // 2|3     4|5         6|11             12|13          14|15
        // шея(2)  руки(3)  спина верх(4)    спина низ(5)     ноги(6)
        //                      |
        //              -------------------
        //             7|8               9|10
        //            arm(7)            abs(8)
        {{wtsv("4")},
         {"1,1,16,"
          "2,2,3,"
          "3,4,5,"
          "4,6,11,"
          "7,7,8,"
          "8,9,10,"
          "5,12,13,"
          "6,14,15,"}},
        //                      тело(1)
        //                      1|18
        //                       |
        //  ------------------------------------------------------
        // 2|3     4|5         6|13             14|15          16|17
        // шея(2)  руки(3)  спина верх(4)    спина низ(5)     ноги(6)
        //                      |
        //              -------------------
        //             7|10             11|12
        //            arm(7)            abs(8)
        //              |
        //             8|9
        //           finger(9)
        {{wtsv("7")},
         {"1,1,18,"
          "2,2,3,"
          "3,4,5,"
          "4,6,13,"
          "7,7,10,"
          "9,8,9,"
          "8,11,12,"
          "5,14,15,"
          "6,16,17,"}},
    };
    size_t i = 0;
    for (const auto _case : cases)
    {
        std::string count_bp_old;
        std::string count_bpl_old;
        std::string count_bpns_old;
        count_initial_entities(count_bp_old, count_bpl_old, count_bpns_old);

        std::string                     msg = fmt::format(" - case #{} - ", i);
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            {wtsv(msg)}, {}, {_case.parent_id}};
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("body_part_insert.sql"),
            .binds_size = BIND_COUNT,
            .binds = binds,
        };
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_OK2(err, msg);

        std::string count_bp_new;
        std::string count_bpl_new;
        std::string count_bpns_new;
        count_initial_entities(count_bp_new, count_bpl_new, count_bpns_new);
        ASSERT_STRNE3(count_bp_old, count_bp_new) << msg;
        ASSERT_STRNE3(count_bpl_old, count_bpl_new) << msg;
        ASSERT_STRNE3(count_bpns_old, count_bpns_new) << msg;
        const auto result_id = wholth_exec_stmt_Result_at(res, 0, 0);
        ASSERT_NE(nullptr, result_id.data) << msg;
        ASSERT_NE(0, result_id.size) << msg;

        ASSERT_TREE(_case.expected_tree, msg);
        i++;
    }
}

TEST_F(Test_wholth_sql_statements_body_part_insert, when_duplicate_title)
{
    const std::string_view    title{"arm"};
    wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
        {wtsv(title)}, {}, {wtsv("1")}};
    wholth_exec_stmt_Args args = {
        .sql_file = wtsv("body_part_insert.sql"),
        .binds_size = BIND_COUNT,
        .binds = binds,
    };

    wholth_StringView initial_id{};
    {
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK(err);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_OK(err);

        initial_id = wholth_exec_stmt_Result_at(res, 0, 0);
        ASSERT_NE(nullptr, initial_id.data);
        ASSERT_NE(0, initial_id.size);
    }

    std::string count_bp_old;
    std::string count_bpl_old;
    std::string count_bpns_old;
    count_initial_entities(count_bp_old, count_bpl_old, count_bpns_old);

    wholth_exec_stmt_Result* res = nullptr;
    auto                     err = wholth_exec_stmt_Result_new(&res);
    ASSERT_ERR_OK(err);
    binds[0].value = wtsv("Arm");
    err = wholth_exec_stmt(&args, res);
    ASSERT_ERR_NOK(err);
    ASSERT_STREQ2("Уже есть часть тела с таким названием!", wfsv(err.message));

    std::string count_bp_new;
    std::string count_bpl_new;
    std::string count_bpns_new;
    count_initial_entities(count_bp_new, count_bpl_new, count_bpns_new);
    ASSERT_STREQ3(count_bp_old, count_bp_new);
    ASSERT_STREQ3(count_bpl_old, count_bpl_new);
    ASSERT_STREQ3(count_bpns_old, count_bpns_new);

    ASSERT_TREE(
        std::string_view{"1,1,14,"
                         "2,2,3,"
                         "3,4,5,"
                         "4,6,7,"
                         "5,8,9,"
                         "6,10,11,"
                         "7,12,13,"},
        " - tree should be unchaged");

    ASSERT_LEAF_DATA(wfsv(initial_id), std::string_view{"arm,,"}, "");
}
