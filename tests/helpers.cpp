#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/app.h"

// sqlw::Connection InMemoryDatabaseAwareTest::db_con {};
//
// sqlw::Connection GlobalInMemoryDatabaseAwareTest::db_con {};

wholth_StringView wtsv(std::string_view sv)
{
    return {sv.data(), sv.size()};
}

std::string_view wfsv(wholth_StringView sv)
{
    if (sv.data == nullptr || sv.size == 0)
    {
        return {};
    }
    return {sv.data, sv.size};
}

void astmt(
    sqlw::Connection& connection,
    std::string_view sql,
    sqlw::Statement::callback_t callback)
{
    sqlw::Statement stmt{&connection};

    std::error_code ec = stmt(sql, callback);
    ASSERT_TRUE(sqlw::status::Condition::OK == ec)
        << ec.value() << ec.message() << sql;
}

// ApplicationAwareSingleton::ApplicationAwareSingleton()
static void init_application()
{
    static bool did_init = false;

    if (did_init)
    {
        return;
    }

    const wholth_AppSetupArgs args{
        .db_path = ":memory:",
    };
    // fmt::print(
    //     fmt::fg(fmt::color::green_yellow),
    //     "ApplicationAwareTest::SetUpTestSuite()\n");
    const auto err = wholth_app_setup(&args);

    if (wholth_Error_OK.code != err.code)
    {
        std::cout << "init application error" << '\n';
        std::cout << '\t' << err.code << '\n';
        std::cout << '\t' << wfsv(err.message) << '\n';
    }

    EXPECT_TRUE(wholth_Error_OK.code == err.code) << err.code << wfsv(err.message);
    EXPECT_TRUE(wholth_Error_OK.message.size == err.message.size) << err.code << wfsv(err.message);

    did_init = true;
}

void ApplicationAwareTest::SetUpTestSuite()
{
    init_application();
}

void ApplicationAwareTest::SetUp()
{
    auto ec = sqlw::Statement{&db::connection()}("SAVEPOINT unittestsp");
    ASSERT_TRUE(sqlw::status::Condition::OK == ec);
}

void ApplicationAwareTest::TearDown()
{
    auto ec = sqlw::Statement{&db::connection()}("ROLLBACK TO unittestsp");
    ASSERT_TRUE(sqlw::status::Condition::OK == ec);
}

void ASSERT_WHOLTH_OK(wholth_Error err, std::string_view msg)
{
    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message) << msg;
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message) << msg;
}

void ASSERT_WHOLTH_NOK(wholth_Error err, std::string_view msg)
{
    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message) << msg;
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message) << msg;
}
