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

class Test_wholth_sql_statements_body_part_update_text
    : public ApplicationAwareTest
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
    Test_wholth_sql_statements_body_part_update_text,
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
        {{wtsv("aRM")}, {}, {"arm,,"}},
        {{wtsv("Leg")}, {wtsv("AN arm")}, {"leg,AN arm,"}},
        {{wtsv("  forearm   ")}, {wtsv(" an arm ")}, {"forearm, an arm ,"}},
    };
    size_t i = 0;
    for (const auto t_case : cases)
    {
        std::string msg = fmt::format(" - case #{} - ", i);
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            {wtsv("2")},
            t_case.title,
            t_case.description,
        };
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("body_part_update_text.sql"),
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

        ASSERT_LEAF_DATA("2", t_case.expected_data, msg);
        ASSERT_LEAF_DATA("1", std::string_view{"тело,,"}, msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});
        i++;
    }
}

TEST_F(Test_wholth_sql_statements_body_part_update_text, when_bad_title)
{
    std::string count_bp_old;
    std::string count_bpl_old;
    std::string count_bpns_old;
    count_initial_entities(count_bp_old, count_bpl_old, count_bpns_old);

    ASSERT_STMT_OK(
        db::connection(),
        "UPDATE body_part_localisation_fts5 SET description='_INITIAL_DESCR_' "
        "WHERE rowid = (SELECT bpl_fts5_rowid FROM body_part WHERE id = 2)",
        [](auto) {});
    struct _Case
    {
        wholth_exec_stmt_Bindable title;
        wholth_exec_stmt_Bindable description;
        std::string_view          expected_data;
    };
    std::vector<_Case> cases = {
        {{wtsv("")}, {}, "шея,_INITIAL_DESCR_,"},
        {{wtsv("     ")}, {}, "шея,_INITIAL_DESCR_,"},
    };
    size_t i = 0;
    for (const auto t_case : cases)
    {
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        std::string                     msg = fmt::format(" - case #{} - ", i);
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            {wtsv("2")},
            t_case.title,
            t_case.description,
        };
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("body_part_update_text.sql"),
            .binds_size = BIND_COUNT,
            .binds = binds,
        };
        wholth_exec_stmt_Result* res = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&res);
        ASSERT_ERR_OK2(err, msg);
        err = wholth_exec_stmt(&args, res);
        ASSERT_ERR_NOK2(err, msg);
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

        ASSERT_LEAF_DATA("2", t_case.expected_data, msg);
        ASSERT_LEAF_DATA("1", std::string_view{"тело,,"}, msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});

        i++;
    }
}

TEST_F(Test_wholth_sql_statements_body_part_update_text, when_null_data)
{
    std::string count_bp_old;
    std::string count_bpl_old;
    std::string count_bpns_old;
    count_initial_entities(count_bp_old, count_bpl_old, count_bpns_old);

    ASSERT_STMT_OK(
        db::connection(),
        "UPDATE body_part_localisation_fts5 SET description='_INITIAL_DESCR_' "
        "WHERE rowid = (SELECT bpl_fts5_rowid FROM body_part WHERE id = 2)",
        [](auto) {});
    struct _Case
    {
        wholth_exec_stmt_Bindable title;
        wholth_exec_stmt_Bindable description;
        std::string_view          expected_data;
    };
    std::vector<_Case> cases = {
        {{}, {}, "шея,_INITIAL_DESCR_,"},
        {{}, {nullptr, 0}, "шея,_INITIAL_DESCR_,"},
        {{wtsv("aboba")}, {wtsv("")}, "aboba,,"},
        {{nullptr, 0}, {}, "шея,_INITIAL_DESCR_,"},
        {{nullptr, 0}, {wtsv("YOOO")}, "шея,YOOO,"},
    };
    size_t i = 0;
    for (const auto t_case : cases)
    {
        ASSERT_STMT_OK(db::connection(), "SAVEPOINT _savepoint_", [](auto) {});
        std::string                     msg = fmt::format(" - case #{} - ", i);
        const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
            {wtsv("2")},
            t_case.title,
            t_case.description,
        };
        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("body_part_update_text.sql"),
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

        ASSERT_LEAF_DATA("2", t_case.expected_data, msg);
        ASSERT_LEAF_DATA("1", std::string_view{"тело,,"}, msg);

        ASSERT_STMT_OK(
            db::connection(), "ROLLBACK TO _savepoint_", [](auto) {});

        i++;
    }
}

TEST_F(Test_wholth_sql_statements_body_part_update_text, when_duplicate_title)
{
    const std::string_view          title{"arm"};
    const wholth_exec_stmt_Bindable binds[BIND_COUNT] = {
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
