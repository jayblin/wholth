#include <gtest/gtest.h>
#include <tuple>
#include "db/db.hpp"
#include "fmt/core.h"
#include "fmt/ranges.h"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "wholth/cmake_vars.h"
#include "wholth/entity/utils.hpp"
#include "wholth/utils.hpp"
#include "wholth/pager.hpp"
#include "wholth/entity/food.hpp"

static void error_log_callback(void *pArg, int iErrCode, const char *zMsg)
{
	std::cout << '[' << iErrCode << "] " << zMsg << '\n';
}

#define ASSERT_STREQ2(a,b) ASSERT_STREQ(a, std::string{b}.data());

GTEST_TEST(NutrientPager, query_page)
{
	sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, nullptr);

	sqlw::Connection db_con {":memory:"};

	db::migration::migrate({
		.con = &db_con,
		.migrations_dir = MIGRATIONS_DIR,
		.filter = [] (const std::filesystem::directory_entry& entry) {
			// Skip migrations that insert values.
			const auto is_insert_migration = std::string_view{entry.path().filename().c_str()}.find("-insert-");

			return is_insert_migration == std::string_view::npos;
		},
		.log = false,
	});

	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'EN')");
	stmt("INSERT INTO locale (id,alias) VALUES (2,'RU')");
	stmt("INSERT INTO locale (id,alias) VALUES (3,'DE')");
	stmt("INSERT INTO nutrient (id,unit,position) "
		"VALUES "
		 "(1001,1,'Solids'),"
		 "(1002,1,'Nitrogen'),"
		 "(1003,1,'Protein'),"
		 "(1004,1,'Total lipid (fat)'),"
		 "(1005,1,'Carbohydrate, by difference'),"
		 "(1007,1,'Ash'),"
		 "(1008,1,'Energy'),"
		 "(1009,1,'Starch')"
	);

	std::array<wholth::entity::nutrient::View, 50> list;
	Pager<wholth::entity::nutrient::View> pager;

	{
		FoodsQuery q {
			.limit = 2,
			.page = 0,
			.locale_id = "1",
		};
		PaginationInfo info = pager.query_page(list, &db_con, q);

		ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[0]));
		ASSERT_STREQ2("Salt", entity::get<entity::food::view::title>(list[0]));
		ASSERT_STREQ2("Essence of salt", entity::get<entity::food::view::description>(list[0]));
		ASSERT_STREQ2("2", entity::get<entity::food::view::id>(list[1]));
		ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::title>(list[1]));
		ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[1]));

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
		auto info = pager.query_page(list, &db_con, q);

		ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[0]));
		ASSERT_STREQ2("Salta", entity::get<entity::food::view::title>(list[0]));
		ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[0]));

		ASSERT_STREQ2("4", entity::get<entity::food::view::id>(list[1]));
		ASSERT_STREQ2("Saltabar", entity::get<entity::food::view::title>(list[1]));
		ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[1]));

		ASSERT_STREQ2("5", entity::get<entity::food::view::id>(list[2]));
		ASSERT_STREQ2("Salia", entity::get<entity::food::view::title>(list[2]));
		ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[2]));

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
		auto info = pager.query_page(list, &db_con, q);

		ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[0]));
		ASSERT_STREQ2("Salt", entity::get<entity::food::view::title>(list[0]));
		ASSERT_STREQ2("Essence of salt", entity::get<entity::food::view::description>(list[0]));

		ASSERT_STREQ2("4", entity::get<entity::food::view::id>(list[1]));
		ASSERT_STREQ2("Saltabar", entity::get<entity::food::view::title>(list[1]));
		ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[1]));

		ASSERT_STREQ2("5", entity::get<entity::food::view::id>(list[2]));
		ASSERT_STREQ2("Salia", entity::get<entity::food::view::title>(list[2]));
		ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[2]));

		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);
	}

	{
		FoodsQuery q {
			.limit = 2,
			.page = 1,
			.locale_id = "2",
			.title = "Sal",
		};
		auto info = pager.query_page(list, &db_con, q);

		// Checks that foods are sorted by id by default.
		ASSERT_STREQ2("5", entity::get<entity::food::view::id>(list[0]));
		ASSERT_STREQ2("Salia", entity::get<entity::food::view::title>(list[0]));
		ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[0]));

		ASSERT_STREQ2("2", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("2/2", info.progress_string);
	}

	{
		std::array<wholth::nutrient_filter::Entry, 500> arr {{
			{wholth::nutrient_filter::Operation::EQ, "1", "100"},
			{wholth::nutrient_filter::Operation::NEQ, "2", "22.4"},
			{wholth::nutrient_filter::Operation::BETWEEN, "3", "10.3,40"},
		}};
		FoodsQuery q {
			.limit = 3,
			.page = 0,
			.locale_id = "",
			.nutrient_filters = arr,
		};
		PaginationInfo info = pager.query_page(list, &db_con, q);

		ASSERT_STREQ2("3", entity::get<entity::food::view::id>(list[0]));
		ASSERT_STREQ2("4", entity::get<entity::food::view::id>(list[1]));
		ASSERT_STREQ2("6", entity::get<entity::food::view::id>(list[2]));

		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);
	}

	// @todo add test cases:
	// - when limit is larger than container size;
}
