#ifndef WHOLTH_TESTS_HELPERS_H_
#define WHOLTH_TESTS_HELPERS_H_

#include "db/db.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/error.h"
#include "wholth/c/pages/utils.h"
#include "wholth/c/string_view.h"
#include <gtest/gtest.h>
#include <string_view>

#define ASSERT_STREQ2(a, b) ASSERT_STREQ(a, std::string{b}.data())
#define ASSERT_STREQ3(a, b) ASSERT_STREQ(a.data(), std::string{b}.data())
// todo rename
#define ASSERT_STRNEQ2(a, b) ASSERT_STRNE(a, std::string{b}.data())
#define ASSERT_STRNE2(a, b) ASSERT_STRNE(a, std::string{b}.data())
#define ASSERT_STRNE3(a, b) ASSERT_STRNE(a.data(), std::string{b}.data())

void ASSERT_WHOLTH_OK(wholth_Error err, std::string_view msg = "");
void ASSERT_WHOLTH_NOK(wholth_Error err, std::string_view msg = "");

class GlobalInMemoryDatabaseAwareTest : public testing::Test
{
  protected:
    static void SetUpTestSuite()
    {
        db::setup_logger();
        db::init(":memory:");
    }
};

class InMemoryDatabaseAwareTest : public testing::Test
{
  protected:
    static void SetUpTestSuite()
    {
        db::setup_logger();
    }

    void SetUp() override
    {
        db::init(":memory:");
    }
};

class GlobalInMemorySavepointWrappedTest
    : public GlobalInMemoryDatabaseAwareTest
{
  protected:
    void SetUp() override
    {
        GlobalInMemoryDatabaseAwareTest::SetUp();
        auto ec = sqlw::Statement{&db::connection()}("SAVEPOINT unittestsp");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
    }

    void TearDown() override
    {
        auto ec = sqlw::Statement{&db::connection()}("ROLLBACK TO unittestsp");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
    }
};

// class ApplicationAwareSingleton
// {
//     public:
//         static auto init() -> const ApplicationAwareSingleton&
//         {
//            static auto g_instance = ApplicationAwareSingleton();
//
//            return g_instance;
//         }
//
//     private:
//        ApplicationAwareSingleton();
// };

class ApplicationAwareTest : public testing::Test
{
  protected:
    static void SetUpTestSuite();
    // {
    //     ApplicationAwareSingleton::init();
    // }

    void SetUp() override;
    // {
    //     auto ec = sqlw::Statement{&db::connection()}("SAVEPOINT unittestsp");
    //     ASSERT_TRUE(sqlw::status::Condition::OK == ec);
    // }

    void TearDown() override;
    // {
    //     auto ec = sqlw::Statement{&db::connection()}("ROLLBACK TO
    //     unittestsp"); ASSERT_TRUE(sqlw::status::Condition::OK == ec);
    // }
};

auto wtsv(std::string_view sv) -> wholth_StringView;
auto wfsv(wholth_StringView sv) -> std::string_view;

auto astmt(
    sqlw::Connection& connection,
    std::string_view sql,
    sqlw::Statement::callback_t callback = nullptr) -> void;

struct PageWrap
{
    wholth_Page* handle = nullptr;

    ~PageWrap()
    {
        wholth_pages_del(handle);
    }
};

#endif // WHOLTH_TESTS_HELPERS_H_
