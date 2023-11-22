#include <gtest/gtest.h>
#include <tuple>
#include "db/db.hpp"
#include "sqlw/connection.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "wholth/cmake_vars.h"
#include "wholth/entity/utils.hpp"
#include "wholth/utils.hpp"
#include "wholth/view_list.hpp"
#include "wholth/entity/food.hpp"

static void error_log_callback(void *pArg, int iErrCode, const char *zMsg)
{
	std::cout << '[' << iErrCode << "] " << zMsg << '\n';
}

#define ASSERT_STREQ2(a,b) ASSERT_STREQ(a, std::string{b}.data());



GTEST_TEST(FoodViewList, test_list)
{
	sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, nullptr);

	sqlw::Connection db_con {":memory:"};

	db::migration::migrate(&db_con, MIGRATIONS_DIR, false);

	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO food (id, created_at, calories) "
		"VALUES (1,'10-10-2010',100),"
		" (2,'10-10-2010',101),"
		" (3,'10-10-2010',102),"
		" (4,'10-10-2010',103),"
		" (5,'10-10-2010',104),"
		" (6,'10-10-2010',105)"
	);
	stmt("INSERT INTO locale (id,alias) VALUES (2,'EN')");
	stmt("INSERT INTO locale (id,alias) VALUES (3,'DE')");
	stmt("INSERT INTO food_localisation (food_id, locale_id, title, description) "
		"VALUES "
		"(1, 1, 'Salt', NULL),"
		"(2, 2, 'Cacao', NULL),"
		"(5, 2, 'Salia', NULL),"
		"(5, 3, 'Saliagr', NULL),"
		"(1, 2, 'Salta', NULL),"
		"(4, 2, 'Saltabar', NULL)"
	);

	ViewList<entity::food::View> list;

	{
		FoodsQuery q {
			.limit = 2,
			.page = 0,
			.locale_id = "1",
		};
		PaginationInfo info = list.query_page(&db_con, q);

		ASSERT_EQ(2, list.size());

		ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[0]));
		ASSERT_STREQ2("Salt", entity::get<entity::food::view::title>(list[0]));
		ASSERT_STREQ2("100", entity::get<entity::food::view::calories>(list[0]));
		ASSERT_STREQ2("2", entity::get<entity::food::view::id>(list[1]));
		ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::title>(list[1]));
		ASSERT_STREQ2("101", entity::get<entity::food::view::calories>(list[1]));

		ASSERT_STREQ2("3", info.max_page);
		ASSERT_STREQ2("6", info.element_count);
		ASSERT_STREQ2("1/3", info.progress_string);
	}

	{
		FoodsQuery q {
			.limit = 10,
			.page = 0,
			.locale_id = "2",
			.title = "Sal",
		};
		auto info = list.query_page(&db_con, q);

		ASSERT_EQ(10, list.size());

		ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[0]));
		ASSERT_STREQ2("Salta", entity::get<entity::food::view::title>(list[0]));
		ASSERT_STREQ2("100", entity::get<entity::food::view::calories>(list[0]));

		ASSERT_STREQ2("4", entity::get<entity::food::view::id>(list[1]));
		ASSERT_STREQ2("Saltabar", entity::get<entity::food::view::title>(list[1]));
		ASSERT_STREQ2("103", entity::get<entity::food::view::calories>(list[1]));

		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);
	}

	{
		FoodsQuery q {
			.limit = 10,
			.page = 0,
			.locale_id = "",
			.title = "Sal",
		};
		auto info = list.query_page(&db_con, q);

		ASSERT_EQ(10, list.size());

		ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[0]));
		ASSERT_STREQ2("Salt", entity::get<entity::food::view::title>(list[0]));
		ASSERT_STREQ2("100", entity::get<entity::food::view::calories>(list[0]));

		ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[1]));
		ASSERT_STREQ2("Salta", entity::get<entity::food::view::title>(list[1]));
		ASSERT_STREQ2("100", entity::get<entity::food::view::calories>(list[1]));

		ASSERT_STREQ2("4", entity::get<entity::food::view::id>(list[2]));
		ASSERT_STREQ2("Saltabar", entity::get<entity::food::view::title>(list[2]));
		ASSERT_STREQ2("103", entity::get<entity::food::view::calories>(list[2]));

		ASSERT_STREQ2("5", entity::get<entity::food::view::id>(list[3]));
		ASSERT_STREQ2("Salia", entity::get<entity::food::view::title>(list[3]));
		ASSERT_STREQ2("104", entity::get<entity::food::view::calories>(list[3]));

		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("5", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);
	}

	{
		FoodsQuery q {
			.limit = 2,
			.page = 1,
			.locale_id = "2",
			.title = "Sal",
		};
		auto info = list.query_page(&db_con, q);

		ASSERT_EQ(2, list.size());

		ASSERT_STREQ2("5", entity::get<entity::food::view::id>(list[0]));
		ASSERT_STREQ2("Salia", entity::get<entity::food::view::title>(list[0]));
		ASSERT_STREQ2("104", entity::get<entity::food::view::calories>(list[0]));

		ASSERT_STREQ2("2", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("2/2", info.progress_string);
	}
}
