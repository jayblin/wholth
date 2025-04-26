#ifndef WHOLTH_TESTS_HELPERS_H_
#define WHOLTH_TESTS_HELPERS_H_

#include "db/db.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "wholth/cmake_vars.h"
#include "wholth/utils.hpp"
#include <gtest/gtest.h>

#define ASSERT_STREQ2(a,b) ASSERT_STREQ(a, std::string{b}.data())
#define ASSERT_STREQ3(a,b) ASSERT_STREQ(a.data(), std::string{b}.data())
#define ASSERT_STRNEQ2(a,b) ASSERT_STRNE(a, std::string{b}.data())

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

class GlobalInMemorySavepointWrappedTest : public GlobalInMemoryDatabaseAwareTest
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

class MigrationAwareTest : public GlobalInMemorySavepointWrappedTest
{
protected:
	static void SetUpTestSuite()
	{
        GlobalInMemoryDatabaseAwareTest::SetUpTestSuite();

		// VERY IMPORTANT!
		auto ec = sqlw::Statement{&db_con}("PRAGMA foreign_keys = ON");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);

		ec = sqlw::Statement{&db_con}("PRAGMA automatic_index = OFF");
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);

		sqlite3_create_function_v2(
			db_con.handle(),
			"seconds_to_readable_time",
			1,
			SQLITE_DETERMINISTIC | SQLITE_DIRECTONLY,
			nullptr,
			wholth::utils::sqlite::seconds_to_readable_time,
			nullptr,
			nullptr,
			nullptr
		);

        ec = db::migration::make_migration_table(&db_con);
        ASSERT_TRUE(db::status::Condition::OK == ec);

        const auto initial_list = db::migration::list_sorted_migrations(MIGRATIONS_DIR);
        std::remove_const_t<decltype(initial_list)> list;

        std::copy_if(
            initial_list.begin(),
            initial_list.end(),
            std::back_inserter(list),
            [](auto i) { return std::string_view{i.path().filename().c_str()}.find("-insert-") == std::string_view::npos; }
        );

		const auto result = db::migration::migrate({
			.con = &db_con,
            .migrations = list,
			.log = false,
		});
        ASSERT_TRUE(db::status::Condition::OK == result.error_code) << result.error_code;
	}
};

#endif // WHOLTH_TESTS_HELPERS_H_
