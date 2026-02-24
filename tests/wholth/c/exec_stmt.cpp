#include "db/db.hpp"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/exec_stmt.h"
#include <string>
#include <unistd.h>
#include <vector>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_c_exec_stmt : public ApplicationAwareTest
{
    void SetUp() override
    {
        ApplicationAwareTest::SetUp();

        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO recipe_step "
            " (id, recipe_id) VALUES "
            " (1, 10),"
            " (2, 10),"
            " (3, 11)");

        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO recipe_step_food "
            " (id, recipe_step_id, food_id) VALUES "
            " (100, 1, 20),"
            " (101, 1, 21),"
            " (102, 2, 22),"
            " (103, 3, 20);");
    }
};

TEST_F(Test_wholth_c_exec_stmt, when_bad_id)
{
    auto& con = db::connection();

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(id) FROM recipe_step_food", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    std::vector<std::string_view> ids = {
        {""},
        {"abc"},
        {"-123"},
        {"1f"},
        {"  102"},
    };
    for (const auto id : ids)
    {
        wholth_Buffer*                  buf = wholth_buffer_ring_pool_element();
        const wholth_exec_stmt_Bindable binds[1] = {{
            .value = wtsv(id),
        }};

        wholth_exec_stmt_Args args = {
            .sql_file_name = wtsv("ingredient_delete.sql"),
            .binds_count = 1,
            .binds = binds,
            .buffer = buf,
        };
        const auto err = wholth_exec_stmt(&args);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        std::string new_count{"bogus"};
        astmt(con, "SELECT COUNT(food_id) FROM recipe_step_food", [&](auto e) {
            new_count = e.column_value;
        });
        ASSERT_STREQ3(old_count, new_count);

        std::string check{"bogus"};
        astmt(
            con,
            "SELECT id FROM recipe_step_food "
            "WHERE id = 102",
            [&](auto e) { check = e.column_value; });
        ASSERT_STREQ2("102", check) << "Record should not be deleted!";
    }
}

TEST_F(Test_wholth_c_exec_stmt, when_bind_count_eq_zero)
{
    auto& con = db::connection();

    std::string old_count{"bogus"};
    astmt(con, "SELECT COUNT(id) FROM recipe_step_food", [&](auto e) {
        old_count = e.column_value;
    });
    ASSERT_NE("bogus", old_count);

    wholth_Buffer*                  buf = wholth_buffer_ring_pool_element();
    const wholth_exec_stmt_Bindable binds[1] = {{
        .value = wtsv("102"),
    }};

    wholth_exec_stmt_Args args = {
        .sql_file_name = wtsv("ingredient_delete.sql"),
        .binds_count = 0,
        .binds = binds,
        .buffer = buf,
    };
    const auto err = wholth_exec_stmt(&args);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::string new_count{"bogus"};
    astmt(con, "SELECT COUNT(food_id) FROM recipe_step_food", [&](auto e) {
        new_count = e.column_value;
    });
    ASSERT_STREQ3(old_count, new_count);

    std::string check{"bogus"};
    astmt(
        con,
        "SELECT id FROM recipe_step_food "
        "WHERE id = 102",
        [&](auto e) { check = e.column_value; });
    ASSERT_STREQ2("102", check) << "Record should not be deleted!";
}
