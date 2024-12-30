#ifndef WHOLTH_TESTS_HELPERS_H_
#define WHOLTH_TESTS_HELPERS_H_

#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include <gtest/gtest.h>

#define ASSERT_STREQ2(a,b) ASSERT_STREQ(a, std::string{b}.data());
#define ASSERT_STREQ3(a,b) ASSERT_STREQ(a.data(), std::string{b}.data());
#define ASSERT_STRNEQ2(a,b) ASSERT_STRNE(a, std::string{b}.data());

static void error_log_callback(void *pArg, int iErrCode, const char *zMsg)
{
	std::cout << '[' << iErrCode << "] " << zMsg << '\n';
}

class GlobalInMemoryDatabaseAwareTest : public testing::Test
{
protected:
	static sqlw::Connection db_con;

	static void SetUpTestSuite()
	{
		sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, nullptr);
		db_con = {":memory:"};
	}
};
sqlw::Connection GlobalInMemoryDatabaseAwareTest::db_con {};

class InMemoryDatabaseAwareTest : public testing::Test
{
protected:
	static sqlw::Connection db_con;

	static void SetUpTestSuite()
	{
		sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, nullptr);
	}

	void SetUp() override
	{
		db_con = {":memory:"};
	}
};
sqlw::Connection InMemoryDatabaseAwareTest::db_con {};

class InMemorySavepointWrappedTest : public GlobalInMemoryDatabaseAwareTest
{
protected:
	void SetUp() override
	{
        GlobalInMemoryDatabaseAwareTest::SetUp();
		auto ec = sqlw::Statement{&db_con}("SAVEPOINT unittestsp");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
	}

	void TearDown() override
	{
		auto ec = sqlw::Statement{&db_con}("ROLLBACK TO unittestsp");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
	}
};

#endif // WHOLTH_TESTS_HELPERS_H_
