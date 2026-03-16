#include "helpers.hpp"
#include "assert.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/exec_stmt.h"
#include <cstdint>
#include <string_view>
#include <unistd.h>
#include <vector>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_sql_statements_nutrient_top_select
    : public ApplicationAwareTest
{
    void SetUp() override
    {
        ApplicationAwareTest::SetUp();

        // ASSERT_STMT_OK(
        //     db::connection(),
        //     "UPDATE nutrient SET position=9999 WHERE 1=1",
        //     [](auto) {});
    }
};

TEST_F(Test_wholth_sql_statements_nutrient_top_select, when_bad_locale_id)
{
    std::vector<std::string_view> ids = {
        {""},
        {"abc"},
        {"-123"},
        {"1f"},
        {"  102"},
    };
    for (const auto id : ids)
    {
        const wholth_exec_stmt_Bindable binds[1] = {{
            .value = wtsv(id),
        }};

        wholth_exec_stmt_Args args = {
            .sql_file = wtsv("nutrient_top_select.sql"),
            .binds_size = 1,
            .binds = binds,
        };

        wholth_exec_stmt_Result* r = nullptr;
        auto                     err = wholth_exec_stmt_Result_new(&r);
        ASSERT_ERR_OK(err);

        err = wholth_exec_stmt(&args, r);
        ASSERT_ERR_NOK(err);
    }
}

TEST_F(Test_wholth_sql_statements_nutrient_top_select, when_good_case)
{
    struct _Nutrient
    {
        std::string_view id;
        std::string_view title;
    };

    const wholth_exec_stmt_Bindable binds[1] = {{
        .value = wtsv("1"),
    }};

    wholth_exec_stmt_Args args = {
        .sql_file = wtsv("nutrient_top_select.sql"),
        .binds_size = 1,
        .binds = binds,
    };

    wholth_exec_stmt_Result* r = nullptr;
    auto                     err = wholth_exec_stmt_Result_new(&r);
    ASSERT_ERR_OK(err);

    err = wholth_exec_stmt(&args, r);
    ASSERT_ERR_OK(err);

    std::vector<_Nutrient> expected_nutrients{{
        {
            "2047",
            "Energy (Atwater General Factors)",
        },
        {
            "2048",
            "Energy (Atwater Specific Factors)",
        },
        {
            "1003",
            "Protein",
        },
        {
            "1053",
            "Adjusted Protein",
        },
        {
            "1004",
            "Total lipid (fat)",
        },
        {
            "1085",
            "Total fat (NLEA)",
        },
        {
            "1005",
            "Carbohydrate, by difference",
        },
        {
            "1050",
            "Carbohydrate, by summation",
        },
        {
            "1072",
            "Carbohydrate, other",
        },
        {
            "2039",
            "Carbohydrates",
        },
        {
            "1063",
            "Sugars, Total",
        },
        {
            "1235",
            "Sugars, added",
        },
        {
            "1236",
            "Sugars, intrinsic",
        },
        {
            "2000",
            "Sugars, total including NLEA",
        },
    }};

    uint64_t row_count = wholth_exec_stmt_Result_row_count(r);
    ASSERT_EQ(expected_nutrients.size(), row_count);

    uint64_t i = 0;
    for (const auto en : expected_nutrients)
    {
        // std::cout << i << '\n';
        // std::cout << en.id << '\n';
        // std::cout << en.title << '\n';
        // std::cout << wfsv(wholth_exec_stmt_Result_at(r, i, 1)) << '\n';

        ASSERT_STREQ3(en.id, wfsv(wholth_exec_stmt_Result_at(r, i, 0)))
            << " " << i;
        ASSERT_STREQ3(en.title, wfsv(wholth_exec_stmt_Result_at(r, i, 1)))
            << " " << i;
        i++;
    }
}
