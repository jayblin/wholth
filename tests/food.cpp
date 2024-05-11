#include <array>
#include <exception>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "db/db.hpp"
#include "fmt/color.h"
#include "fmt/core.h"
#include "fmt/ranges.h"
#include "sqlw/connection.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "sqlw/status.hpp"
#include "wholth/cmake_vars.h"
#include "wholth/entity/utils.hpp"
#include "wholth/list/food.hpp"
#include "wholth/utils.hpp"
#include "wholth/pager.hpp"
#include "wholth/entity/food.hpp"

static void error_log_callback(void *pArg, int iErrCode, const char *zMsg)
{
	std::cout << '[' << iErrCode << "] " << zMsg << '\n';
}

#define ASSERT_STREQ2(a,b) ASSERT_STREQ(a, std::string{b}.data());
#define ASSERT_STREQ3(a,b) ASSERT_STREQ(a.data(), std::string{b}.data());
#define ASSERT_STRNEQ2(a,b) ASSERT_STRNE(a, std::string{b}.data());

class MigrationAwareTest : public testing::Test
{
protected:
	static sqlw::Connection db_con;

	static void SetUpTestSuite()
	{
		sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, nullptr);

		db_con = {":memory:"};

		// VERY IMPORTANT!
		sqlw::Statement{&db_con}("PRAGMA foreign_keys = ON");
		/* sqlw::Statement{&db_con}("PRAGMA automatic_index = OFF"); */
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
	}

	static void TearDownTestSuite()
	{
	}

	void SetUp() override
	{
		sqlw::Statement{&db_con}("SAVEPOINT unittestsp");
	}

	void TearDown() override
	{
		sqlw::Statement{&db_con}("ROLLBACK TO unittestsp");
	}
};

sqlw::Connection MigrationAwareTest::db_con;

TEST_F(MigrationAwareTest, list_foods)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		" (1,'10-10-2010'),"
		" (2,'10-10-2010'),"
		" (3,'10-10-2010'),"
		" (4,'10-10-2010'),"
		" (5,'10-10-2010'),"
		" (6,'10-10-2010')"
	);
	stmt(
		"INSERT INTO locale (id,alias) VALUES "
		"(1,'EN'),(2,'RU'),(3,'DE')"
	);
	stmt("INSERT INTO food_localisation (food_id, locale_id, title, description) "
		"VALUES "
		"(1, 1, 'Salt', 'Essence of salt'),"
		"(2, 2, 'Cacao', NULL),"
		"(5, 2, 'Salia', NULL),"
		"(5, 3, 'Saliagr', 'I dont''t know'),"
		"(1, 2, 'Salta', NULL),"
		"(4, 2, 'Saltabar', NULL)"
	);
	stmt("INSERT INTO nutrient (id,unit,position) "
		"VALUES "
		"(1, 'a', 1),"
		"(2, 'b', 0),"
		"(3, 'c', 2)"
	);
	stmt("INSERT INTO food_nutrient (food_id, nutrient_id, value) "
		"VALUES "
		"(1, 1, 100)," // -
		"(1, 2, 22.4),"
		"(1, 3, 20),"
		"(2, 1, 99)," // -
		"(2, 2, 19),"
		"(2, 3, 20),"
		"(3, 1, 100)," // +
		"(3, 2, 20),"
		"(3, 3, 10.33),"
		"(4, 1, 100)," // +
		"(4, 2, 30),"
		"(4, 3, 39.999),"
		"(5, 1, 10)," // -
		"(5, 2, 20),"
		"(5, 3, 20),"
		"(6, 1, 100)," // +
		"(6, 2, 0),"
		"(6, 3, 30)"
	);
	stmt("INSERT INTO recipe_step (recipe_id,seconds) VALUES "
		"(1, '600') "
	);

	std::string buffer1 {};
	std::string buffer2 {};
	std::string buffer3 {};

	{
		std::array<wholth::entity::shortened::Food, 2> list;
		wholth::list::food::Query q {
			.page = 0,
			.locale_id = "1",
		};
		wholth::list::food::Lister lister {q, &db_con};

		wholth::StatusCode rc1 = lister.list(
			list,
			buffer1
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc1) << wholth::view(rc1);
		ASSERT_STREQ2("1", list[0].id);
		ASSERT_STREQ2("Salt", list[0].title);
		ASSERT_STREQ2("10m", list[0].preparation_time);
		ASSERT_STREQ2("2", list[1].id);
		ASSERT_STREQ2("[N/A]", list[1].title);
		ASSERT_STREQ2("[N/A]", list[1].preparation_time);

		PaginationInfo info;
		wholth::StatusCode rc2 = lister.pagination(
			info,
			buffer2,
			list.size()
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc2) << wholth::view(rc2);
		ASSERT_STREQ2("3", info.max_page);
		ASSERT_STREQ2("6", info.element_count);
		ASSERT_STREQ2("1/3", info.progress_string);

		wholth::StatusCode rc3 = lister.count(buffer3);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc3) << wholth::view(rc3);
		ASSERT_STREQ2("6", buffer3);
	}

	{
		std::array<wholth::entity::shortened::Food, 10> list;
		wholth::list::food::Query q {
			.page = 0,
			.locale_id = "2",
			.title = "Sal",
		};
		wholth::list::food::Lister lister {q, &db_con};

		wholth::StatusCode rc1 = lister.list(
			list,
			buffer1
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc1) << wholth::view(rc1);
		ASSERT_STREQ2("1", list[0].id);
		ASSERT_STREQ2("Salta", list[0].title);
		ASSERT_STREQ2("10m", list[0].preparation_time);
		ASSERT_STREQ2("4", list[1].id);
		ASSERT_STREQ2("Saltabar", list[1].title);
		ASSERT_STREQ2("[N/A]", list[1].preparation_time);
		ASSERT_STREQ2("5", list[2].id);
		ASSERT_STREQ2("Salia", list[2].title);
		ASSERT_STREQ2("[N/A]", list[2].preparation_time);

		PaginationInfo info;
		wholth::StatusCode rc2 = lister.pagination(
			info,
			buffer2,
			list.size()
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc2) << wholth::view(rc2);
		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);

		wholth::StatusCode rc3 = lister.count(buffer3);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc3) << wholth::view(rc3);
		ASSERT_STREQ2("3", buffer3);
	}

	{
		std::array<wholth::entity::shortened::Food, 10> list;
		wholth::list::food::Query q {
			.page = 0,
			.locale_id = "",
			.title = "Sal",
		};
		wholth::list::food::Lister lister {q, &db_con};

		wholth::StatusCode rc1 = lister.list(
			list,
			buffer1
		);
		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc1) << wholth::view(rc1);

		PaginationInfo info;
		wholth::StatusCode rc2 = lister.pagination(
			info,
			buffer2,
			list.size()
		);
		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc2) << wholth::view(rc2);

		wholth::StatusCode rc3 = lister.count(buffer3);
		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc3) << wholth::view(rc3);
	}

	{
		std::array<wholth::entity::shortened::Food, 10> list;
		wholth::list::food::Query q {
			.page = 0,
			.locale_id = "1a",
			.title = "Sal",
		};
		wholth::list::food::Lister lister {q, &db_con};

		wholth::StatusCode rc1 = lister.list(
			list,
			buffer1
		);
		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc1) << wholth::view(rc1);

		PaginationInfo info;

		wholth::StatusCode rc2 = lister.pagination(
			info,
			buffer2,
			list.size()
		);
		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc2) << wholth::view(rc2);

		wholth::StatusCode rc3 = lister.count(buffer3);
		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc3) << wholth::view(rc3);
	}

	{
		std::array<wholth::entity::shortened::Food, 2> list;
		wholth::list::food::Query q {
			.page = 1,
			.locale_id = "2",
			.title = "Sal",
		};
		wholth::list::food::Lister lister {q, &db_con};

		wholth::StatusCode rc1 = lister.list(
			list,
			buffer1
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc1) << wholth::view(rc1);
		// Checks that foods are sorted by id by default.
		ASSERT_STREQ2("5", list[0].id);
		ASSERT_STREQ2("Salia", list[0].title);
		ASSERT_STREQ2("[N/A]", list[0].preparation_time);

		PaginationInfo info;
		wholth::StatusCode rc2 = lister.pagination(
			info,
			buffer2,
			list.size()
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc2) << wholth::view(rc2);
		ASSERT_STREQ2("2", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("2/2", info.progress_string);

		wholth::StatusCode rc3 = lister.count(buffer3);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc3) << wholth::view(rc3);
		ASSERT_STREQ2("3", buffer3);
	}

	{
		std::array<wholth::list::food::nutrient_filter::Entry, 500> arr {{
			{wholth::list::food::nutrient_filter::Operation::EQ, "1", "100"},
			{wholth::list::food::nutrient_filter::Operation::NEQ, "2", "22.4"},
			{wholth::list::food::nutrient_filter::Operation::BETWEEN, "3", "10.3,40"},
		}};
		std::array<wholth::entity::shortened::Food, 3> list;
		wholth::list::food::Query q {
			.page = 0,
			.locale_id = "1",
			.nutrient_filters = arr,
		};
		wholth::list::food::Lister lister {q, &db_con};

		wholth::StatusCode rc1 = lister.list(
			list,
			buffer1
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc1) << wholth::view(rc1);
		ASSERT_STREQ2("3", list[0].id);
		ASSERT_STREQ2("4", list[1].id);
		ASSERT_STREQ2("6", list[2].id);

		PaginationInfo info;
		wholth::StatusCode rc2 = lister.pagination(
			info,
			buffer2,
			list.size()
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc2) << wholth::view(rc2);
		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);

		wholth::StatusCode rc3 = lister.count(buffer3);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc3) << wholth::view(rc3);
		ASSERT_STREQ2("3", buffer3);
	}

	// switch buffer check
	{
		std::array<wholth::entity::shortened::Food, 4> list;
		std::string buffer1_1 {};
		std::string buffer1_2 {};
		wholth::list::food::Query q {
			.page = 0,
			.locale_id = "1",
		};

		wholth::list::food::Lister lister {q, &db_con};

		wholth::StatusCode rc1 = lister.list(
			std::span{list.begin(), 2},
			buffer1_1
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc1);
		ASSERT_STREQ2("1", list[0].id);
		ASSERT_STREQ2("Salt", list[0].title);
		ASSERT_STREQ2("10m", list[0].preparation_time);
		ASSERT_STREQ2("2", list[1].id);
		ASSERT_STREQ2("[N/A]", list[1].title);
		ASSERT_STREQ2("[N/A]", list[1].preparation_time);

		q.page = 1;
		wholth::StatusCode rc2 = lister.list(
			std::span{list.begin() + 2, 2},
			buffer1_1
		);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc2);
		ASSERT_STREQ2("3", list[2].id);
		ASSERT_STREQ2("[N/A]", list[2].title);
		ASSERT_STREQ2("[N/A]", list[2].preparation_time);
		ASSERT_STREQ2("4", list[3].id);
		ASSERT_STREQ2("[N/A]", list[3].title);
		ASSERT_STREQ2("[N/A]", list[3].preparation_time);
	}
}

TEST_F(MigrationAwareTest, list_foods_by_ingredients)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		" (1,'10-10-2010'),"
		" (2,'10-10-2010'),"
		" (3,'10-10-2010'),"
		" (4,'10-10-2010'),"
		" (5,'10-10-2010'),"
		" (6,'10-10-2010'),"
		" (7,'10-10-2010')"
	);
	stmt(
		"INSERT INTO locale (id,alias) VALUES "
		"(1,'EN'),(2,'RU'),(3,'DE')"
	);
	stmt("INSERT INTO food_localisation (food_id, locale_id, title) "
		"VALUES "
		"(1, 3, 'scrambled eggs'), "
		"(2, 3, 'eggs'), "
		"(3, 3, 'salt'), "
		"(4, 3, 'butter'), "
		"(5, 3, 'sugar'), "
		"(6, 3, 'water'), "
		"(7, 3, 'soda') "
	);
	// todo case when recipe contains it's ingredient name.
	stmt("INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
		"(100, 1, '3720'), "
		"(200, 1, '60'), "
		"(300, 7, '340') "
	);
	stmt("INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) VALUES "
		"(100, 2, 21), "
		"(200, 3, 31), "
		"(200, 4, 41), "
		"(300, 5, 52), "
		"(300, 6, 62) "
	);

	{
		std::string buffer1;
		std::array<wholth::entity::shortened::Food, 10> list;
		wholth::list::food::Query q {
			.page = 0,
			.locale_id = "3",
			.ingredients = "water,salt",
		};
		/* PaginationInfo info; */
		wholth::list::food::Lister lister {q, &db_con};

		wholth::StatusCode rc1 = lister.list(
			list,
			buffer1
		);

		/* PaginationInfo info; */
		/* wholth::StatusCode rc2 = lister.pagination( */
		/* 	info, */
		/* 	buffer2, */
		/* 	list.size() */
		/* ); */
		/* wholth::StatusCode rc3 = lister.count(buffer3); */

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc1) << wholth::view(rc1);
		/* ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc2) << wholth::view(rc2); */
		/* ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc3) << wholth::view(rc3); */

		/* ASSERT_STREQ2("1", list[0].id); */
		/* ASSERT_STREQ2("Salta", list[0].title); */
		/* ASSERT_STREQ2("10m", list[0].preparation_time); */

		/* ASSERT_STREQ2("4", list[1].id); */
		/* ASSERT_STREQ2("Saltabar", list[1].title); */
		/* ASSERT_STREQ2("[N/A]", list[1].preparation_time); */

		/* ASSERT_STREQ2("5", list[2].id); */
		/* ASSERT_STREQ2("Salia", list[2].title); */
		/* ASSERT_STREQ2("[N/A]", list[2].preparation_time); */

		/* ASSERT_STREQ2("1", info.max_page); */
		/* ASSERT_STREQ2("3", info.element_count); */
		/* ASSERT_STREQ2("1/1", info.progress_string); */
	}
}

TEST_F(MigrationAwareTest, insert_food)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'EN'),(2,'GB')");

	wholth::entity::editable::Food food {
		.id = "22",
		.title = " Tomato   ",
		.description = "A red thing",
	};
	std::string food_id;
	wholth::StatusCode rc = wholth::insert_food(food, food_id, db_con, "2");

	ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);

	std::string id {"bogus"};
	// Record is created in `food` table.
	{
		stmt(
			"SELECT id FROM food",
			[&] (auto e) {
				id = e.column_value;
			}
		);
		ASSERT_STRNEQ2("bogus", id);
		ASSERT_STREQ3(food_id, id);
	}

	// Title and description are saved.
	{
		std::string title {"bogus"};
		std::string description {"bogus"};
		stmt(
			fmt::format(
				"SELECT title, description "
				"FROM food_localisation "
				"WHERE food_id = {} AND locale_id = 2",
				food_id
			),
			[&] (auto e) {
				if (e.column_name == "title") {
					title = e.column_value;
				}
				else {
					description = e.column_value;
				}
			}
		);
		ASSERT_STREQ2("tomato", title);
		ASSERT_STREQ2("A red thing", description);
	}
}

TEST_F(MigrationAwareTest, insert_food_when_locale_id_is_bogus)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'EN'),(2,'GB')");

	wholth::entity::editable::Food food {
		.title = "chilli pepper",
		.description = "A red thing",
	};
	std::string food_id;
	wholth::StatusCode rc = wholth::insert_food(food, food_id, db_con, "");

	ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc) << wholth::view(rc);

	std::string id {"bogus"};
	// Record is created in `food` table.
	{
		stmt(
			"SELECT id FROM food",
			[&] (auto e) {
				id = e.column_value;
			}
		);
		ASSERT_STREQ2("bogus", id);
	}

	// Title and description are saved.
	{
		std::string count {"bogus"};
		stmt(
			fmt::format(
				"SELECT count(title) "
				"FROM food_localisation ",
				id
			),
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("0", count);
	}
}

TEST_F(MigrationAwareTest, insert_food_when_duplicate)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'EN'),(2,'GB')");
	stmt("INSERT INTO food (id,created_at) VALUES (10,'whatever')");
	stmt(
		"INSERT INTO food_localisation (food_id,locale_id,title,description) "
		"VALUES (10,2,'acai','a fruit')"
	);

	wholth::entity::editable::Food food {
		.title = "acai",
		.description = "A red thing",
	};
	std::string food_id;
	wholth::StatusCode rc = wholth::insert_food(food, food_id, db_con, "2");

	ASSERT_EQ(wholth::StatusCode::SQL_STATEMENT_ERROR, rc) << wholth::view(rc);

	std::string id {"bogus"};
	// Record is created in `food` table.
	{
		stmt(
			"SELECT MAX(id) FROM food",
			[&] (auto e) {
				id = e.column_value;
			}
		);
		ASSERT_STREQ2("10", id);
	}

	// Title and description are saved.
	{
		std::string title {"bogus"};
		std::string description {"bogus"};
		stmt(
			"SELECT title, description FROM food_localisation WHERE food_id = 10",
			[&] (auto e) {
				if (e.column_name == "title") {
					title = e.column_value;
				}
				else {
					description = e.column_value;
				}
			}
		);
		ASSERT_STREQ2("acai", title);
		ASSERT_STREQ2("a fruit", description);

		std::string count {"bogus"};
		stmt(
			"SELECT count(title) FROM food_localisation",
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("1", count);
	}
}

TEST_F(MigrationAwareTest, update_food)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (3,'EN'),(2,'GB')");
	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		"(4,'a date')");
	stmt("INSERT INTO food_localisation (food_id,locale_id,title,description) "
		"VALUES "
		"(4,2,'food 4','a food 4 GB'),"
		"(4,3,'food 4','a food 4 EN')"
	);

	// Title and description are updated only for the specified food.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.title="  Salt ",
			.description="for nerves",
		};
		wholth::UpdateFoodStatus errors = wholth::update_food(food, db_con, "3");

		std::vector<std::string> title {};
		std::vector<std::string> description {};
		stmt(
			"SELECT title, description "
			"FROM food_localisation "
			"WHERE food_id = 4 "
			"ORDER BY locale_id ASC",
			[&] (auto e) {
				if (e.column_name == "title") {
					title.emplace_back(e.column_value);
				}
				else {
					description.emplace_back(e.column_value);
				}
			}
		);
		ASSERT_EQ(2, title.size());
		ASSERT_EQ(2, description.size());
		ASSERT_STREQ2("food 4", title[0]);
		ASSERT_STREQ2("salt", title[1]);
		ASSERT_STREQ2("a food 4 GB", description[0]);
		ASSERT_STREQ2("for nerves", description[1]);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.title) << wholth::view(errors.title);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.description) << wholth::view(errors.description);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.rc) << wholth::view(errors.rc);
	}

	// Only title is updated for the specified food.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.title="Salti",
		};
		wholth::UpdateFoodStatus errors = wholth::update_food(food, db_con, "3");

		std::vector<std::string> title {};
		std::vector<std::string> description {};
		stmt(
			"SELECT title, description "
			"FROM food_localisation "
			"WHERE food_id = 4 AND locale_id = 3",
			[&] (auto e) {
				if (e.column_name == "title") {
					title.emplace_back(e.column_value);
				}
				else {
					description.emplace_back(e.column_value);
				}
			}
		);
		ASSERT_EQ(1, title.size());
		ASSERT_EQ(1, description.size());
		ASSERT_STREQ2("salti", title[0]);
		ASSERT_STREQ2("for nerves", description[0]);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.title) << wholth::view(errors.title);
		ASSERT_EQ(wholth::StatusCode::UNCHANGED_FOOD_DESCRIPTION, errors.description) << wholth::view(errors.description);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.rc) << wholth::view(errors.rc);
	}

	// Only description is updated for the specified food.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.description="Salti is salty",
		};
		wholth::UpdateFoodStatus errors = wholth::update_food(food, db_con, "3");

		std::vector<std::string> title {};
		std::vector<std::string> description {};
		stmt(
			"SELECT title, description "
			"FROM food_localisation "
			"WHERE food_id = 4 AND locale_id = 3 ",
			[&] (auto e) {
				if (e.column_name == "title") {
					title.emplace_back(e.column_value);
				}
				else {
					description.emplace_back(e.column_value);
				}
			}
		);
		ASSERT_EQ(1, title.size());
		ASSERT_EQ(1, description.size());
		ASSERT_STREQ2("salti", title[0]);
		ASSERT_STREQ2("Salti is salty", description[0]);
		ASSERT_EQ(wholth::StatusCode::UNCHANGED_FOOD_TITLE, errors.title) << wholth::view(errors.title);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.description) << wholth::view(errors.description);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.rc) << wholth::view(errors.rc);
	}

	// Title is an empty string.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.title="",
		};
		wholth::UpdateFoodStatus errors = wholth::update_food(food, db_con, "3");

		std::vector<std::string> title {};
		std::vector<std::string> description {};
		stmt(
			"SELECT title, description "
			"FROM food_localisation "
			"WHERE food_id = 4 AND locale_id = 3 "
			"ORDER BY locale_id ASC",
			[&] (auto e) {
				if (e.column_name == "title") {
					title.emplace_back(e.column_value);
				}
				else {
					description.emplace_back(e.column_value);
				}
			}
		);
		ASSERT_EQ(1, title.size());
		ASSERT_EQ(1, description.size());
		ASSERT_STREQ2("salti", title[0]);
		ASSERT_STREQ2("Salti is salty", description[0]);
		ASSERT_EQ(wholth::StatusCode::EMPTY_FOOD_TITLE, errors.title) << wholth::view(errors.title);
		ASSERT_EQ(wholth::StatusCode::UNCHANGED_FOOD_DESCRIPTION, errors.description) << wholth::view(errors.description);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.rc) << wholth::view(errors.rc);
	}

	// Title is nothing but spaces.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.title="   ",
		};
		wholth::UpdateFoodStatus errors = wholth::update_food(food, db_con, "3");

		std::vector<std::string> title {};
		std::vector<std::string> description {};
		stmt(
			"SELECT title, description "
			"FROM food_localisation "
			"WHERE food_id = 4 AND locale_id = 3 "
			"ORDER BY locale_id ASC",
			[&] (auto e) {
				if (e.column_name == "title") {
					title.emplace_back(e.column_value);
				}
				else {
					description.emplace_back(e.column_value);
				}
			}
		);
		ASSERT_EQ(1, title.size());
		ASSERT_EQ(1, description.size());
		ASSERT_STREQ2("salti", title[0]);
		ASSERT_STREQ2("Salti is salty", description[0]);
		ASSERT_EQ(wholth::StatusCode::EMPTY_FOOD_TITLE, errors.title) << wholth::view(errors.title);
		ASSERT_EQ(wholth::StatusCode::UNCHANGED_FOOD_DESCRIPTION, errors.description) << wholth::view(errors.description);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.rc) << wholth::view(errors.rc);
	}

	// Description is an empty string.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.description="",
		};
		wholth::UpdateFoodStatus errors = wholth::update_food(food, db_con, "3");

		std::vector<std::string> title {};
		std::vector<std::string> description {};
		stmt(
			"SELECT title, description "
			"FROM food_localisation "
			"WHERE food_id = 4 AND locale_id = 3 ",
			[&] (auto e) {
				if (e.column_name == "title") {
					title.emplace_back(e.column_value);
				}
				else {
					description.emplace_back(e.column_value);
				}
			}
		);
		ASSERT_EQ(1, title.size());
		ASSERT_EQ(1, description.size());
		ASSERT_STREQ2("salti", title[0]);
		ASSERT_STREQ2("", description[0]);
		ASSERT_EQ(wholth::StatusCode::UNCHANGED_FOOD_TITLE, errors.title) << wholth::view(errors.title);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.description) << wholth::view(errors.description);
		ASSERT_EQ(wholth::StatusCode::NO_ERROR, errors.rc) << wholth::view(errors.rc);
	}
}

TEST_F(MigrationAwareTest, update_food_when_no_locale_id)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (3,'EN'),(2,'GB')");
	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		"(4,'a date')");
	stmt("INSERT INTO food_localisation (food_id,locale_id,title,description) "
		"VALUES "
		"(4,2,'food 4','a food 4 GB'),"
		"(4,3,'food 4','a food 4 EN')"
	);

	wholth::entity::editable::Food food {
		.id="4",
		.title="  Salt ",
		.description="for nerves",
	};
	wholth::update_food(food, db_con, "");

	std::vector<std::string> title {};
	std::vector<std::string> description {};
	stmt(
		"SELECT title, description "
		"FROM food_localisation "
		"WHERE food_id = 4 "
		"ORDER BY locale_id ASC",
		[&] (auto e) {
			if (e.column_name == "title") {
				title.emplace_back(e.column_value);
			}
			else {
				description.emplace_back(e.column_value);
			}
		}
	);
	ASSERT_EQ(2, title.size());
	ASSERT_EQ(2, description.size());
	ASSERT_STREQ2("food 4", title[0]);
	ASSERT_STREQ2("food 4", title[1]);
	ASSERT_STREQ2("a food 4 GB", description[0]);
	ASSERT_STREQ2("a food 4 EN", description[1]);
}

TEST_F(MigrationAwareTest, update_food_when_mishapen_food_id)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (3,'EN'),(2,'GB')");
	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		"(4,'a date')");
	stmt("INSERT INTO food_localisation (food_id,locale_id,title,description) "
		"VALUES "
		"(4,2,'food 4','a food 4 GB'),"
		"(4,3,'food 4','a food 4 EN')"
	);

	{
		wholth::entity::editable::Food food {
			.id="",
			.title="  Salt ",
			.description="for nerves",
		};
		wholth::update_food(food, db_con, "3");

		std::vector<std::string> title {};
		std::vector<std::string> description {};
		stmt(
			"SELECT title, description "
			"FROM food_localisation "
			"WHERE food_id = 4 "
			"ORDER BY locale_id ASC",
			[&] (auto e) {
				if (e.column_name == "title") {
					title.emplace_back(e.column_value);
				}
				else {
					description.emplace_back(e.column_value);
				}
			}
		);
		ASSERT_EQ(2, title.size());
		ASSERT_EQ(2, description.size());
		ASSERT_STREQ2("food 4", title[0]);
		ASSERT_STREQ2("food 4", title[1]);
		ASSERT_STREQ2("a food 4 GB", description[0]);
		ASSERT_STREQ2("a food 4 EN", description[1]);
	}

	{
		wholth::entity::editable::Food food {
			.title="  Salt ",
			.description="for nerves",
		};
		wholth::update_food(food, db_con, "3");

		std::vector<std::string> title {};
		std::vector<std::string> description {};
		stmt(
			"SELECT title, description "
			"FROM food_localisation "
			"WHERE food_id = 4 "
			"ORDER BY locale_id ASC",
			[&] (auto e) {
				if (e.column_name == "title") {
					title.emplace_back(e.column_value);
				}
				else {
					description.emplace_back(e.column_value);
				}
			}
		);
		ASSERT_EQ(2, title.size());
		ASSERT_EQ(2, description.size());
		ASSERT_STREQ2("food 4", title[0]);
		ASSERT_STREQ2("food 4", title[1]);
		ASSERT_STREQ2("a food 4 GB", description[0]);
		ASSERT_STREQ2("a food 4 EN", description[1]);
	}

	{
		wholth::entity::editable::Food food {
			.id="117",
			.title="corn",
			.description="for popping",
		};
		wholth::update_food(food, db_con, "3");

		std::string count {};
		stmt(
			"SELECT count(title) "
			"FROM food_localisation",
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("2", count);
	}
}

TEST_F(MigrationAwareTest, remove_food)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'EN'),(2,'GB')");
	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date'),"
		"(2,'a date'),"
		"(3,'a date'),"
		"(4,'a date')");
	stmt("INSERT INTO food_localisation (food_id,locale_id,title,description) "
		"VALUES (2,1,'food 2','a food 2'),"
		"(3,2,'food 3','a food 3'),"
		"(3,1,'food 3','a food 3')"
	);
	stmt("INSERT INTO nutrient (id,unit,position) "
		"VALUES "
		"(1, 'kcal', 3),"
		"(2, 'mg', 1),"
		"(5, 'mg', 2)"
	);
	stmt("INSERT INTO nutrient_localisation (nutrient_id,title,locale_id) "
		"VALUES "
		"(1, 'Energy', 1),"
		"(2, 'Fats', 1),"
		"(5, 'Proteins', 1)"
	);
	stmt("INSERT INTO food_nutrient (food_id,nutrient_id,value) "
		"VALUES "
		"(1,1,10.1),"
		"(3,2,30),"
		"(2,2,9.333),"
		"(2,5,4)"
	);
	stmt(
		"INSERT INTO recipe_step (id,recipe_id,seconds) "
		"VALUES "
		"(1,1,40),"
		"(2,3,10)"
	);
	stmt(
		"INSERT INTO recipe_step_localisation (recipe_step_id,locale_id,description) "
		"VALUES "
		"(1,1,'Step 1 for food 2'),"
		"(2,1,'Step 1 for food 3')"
	);
	stmt(
		"INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) "
		"VALUES "
		"(1,2,70),"
		"(2,3,90)"
	);

	wholth::remove_food("3", db_con);

	std::string id = "bogus";
	// Record is removed from `food` table.
	{
		stmt(
			"SELECT id FROM food WHERE id = 3",
			[&] (auto e) {
				id = e.column_value;
			}
		);
		ASSERT_STREQ2("bogus", id);
	}

	// Removed food's localisation info is deleted.
	{
		std::string food_loc_count;
		stmt(
			"SELECT count(food_id) FROM food_localisation WHERE food_id = 3",
			[&] (auto e) {
				food_loc_count = e.column_value;
			}
		);
		ASSERT_STREQ2("0", food_loc_count);
	}

	// Localisation info of other foods is intact.
	{
		std::string food_loc_count;
		stmt(
			"SELECT count(food_id) FROM food_localisation",
			[&] (auto e) {
				food_loc_count = e.column_value;
			}
		);
		ASSERT_STREQ2("1", food_loc_count);
	}

	// Removed food's nutrient info is deleted.
	{
		std::string nutrient_count;
		stmt(
			"SELECT count(food_id) FROM food_nutrient WHERE food_id = 3",
			[&] (auto e) {
				nutrient_count = e.column_value;
			}
		);
		ASSERT_STREQ2("0", nutrient_count);
	}

	// Nutrient info of other foods is intact.
	{
		std::string nutrient_count;
		stmt(
			"SELECT count(food_id) FROM food_nutrient",
			[&] (auto e) {
				nutrient_count = e.column_value;
			}
		);
		ASSERT_STREQ2("3", nutrient_count);
	}

	// Removed recipe's directions are removed.
	{
		std::string step_count;
		stmt(
			"SELECT count(recipe_id) FROM recipe_step WHERE recipe_id = 3",
			[&] (auto e) {
				step_count = e.column_value;
			}
		);
		ASSERT_STREQ2("0", step_count);
	}

	// Directions of other recipes are intact.
	{
		std::string step_count;
		stmt(
			"SELECT count(recipe_id) FROM recipe_step",
			[&] (auto e) {
				step_count = e.column_value;
			}
		);
		ASSERT_STREQ2("1", step_count);
	}

	// Localised directions of scpecified recipe is removed.
	{
		std::string step_loc_count;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation WHERE recipe_step_id = 2",
			[&] (auto e) {
				step_loc_count = e.column_value;
			}
		);
		ASSERT_STREQ2("0", step_loc_count);
	}

	// Localised directions for other recipes is not deleted.
	{
		std::string step_loc_count;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				step_loc_count = e.column_value;
			}
		);
		ASSERT_STREQ2("1", step_loc_count);
	}

	// Ingredients associated with specified recipe are removed.
	{
		std::string step_food_count;
		stmt(
			"SELECT count(food_id) FROM recipe_step_food WHERE recipe_step_id = 2",
			[&] (auto e) {
				step_food_count = e.column_value;
			}
		);
		ASSERT_STREQ2("0", step_food_count);
	}

	// Ingredients associated with other recipes are not removed.
	{
		std::string step_food_count;
		stmt(
			"SELECT count(food_id) FROM recipe_step_food",
			[&] (auto e) {
				step_food_count = e.column_value;
			}
		);
		ASSERT_STREQ2("1", step_food_count);
	}
}

TEST_F(MigrationAwareTest, remove_food_when_mishapen_food_id)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'EN'),(2,'GB')");
	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date'),"
		"(2,'a date'),"
		"(3,'a date'),"
		"(4,'a date')");

	{
		wholth::StatusCode rc = wholth::remove_food("", db_con);

		ASSERT_EQ(wholth::StatusCode::INVALID_FOOD_ID, rc) << wholth::view(rc);

		std::string count = "";
		stmt(
			"SELECT COUNT(id) FROM food",
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("4", count);
	}

	{
		wholth::StatusCode rc = wholth::remove_food("   ", db_con);

		ASSERT_EQ(wholth::StatusCode::INVALID_FOOD_ID, rc) << wholth::view(rc);

		std::string count = "";
		stmt(
			"SELECT COUNT(id) FROM food",
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("4", count);
	}

	{
		wholth::StatusCode rc = wholth::remove_food("22", db_con);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);

		std::string count = "";
		stmt(
			"SELECT COUNT(id) FROM food",
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("4", count);
	}

	{
		wholth::StatusCode rc = wholth::remove_food(" 3  ", db_con);

		ASSERT_EQ(wholth::StatusCode::INVALID_FOOD_ID, rc) << wholth::view(rc);

		std::string count = "";
		stmt(
			"SELECT COUNT(id) FROM food",
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("4", count);
	}
}

TEST_F(MigrationAwareTest, add_steps)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (3,'EN'),(2,'GB')");
	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date')");

	std::vector<wholth::entity::editable::food::RecipeStep> steps {{
		{.seconds="200", .description="1 - make cheese; 2 - eat cheese;"},
	}};

	wholth::add_steps("1", steps, db_con, "2");

	std::string step_id {"bogus"};
	// Steps are added to the recipe.
	{
		std::string seconds;
		stmt(
			"SELECT id, seconds "
			"FROM recipe_step "
			"WHERE recipe_id = 1",
			[&] (auto e) {
				if (e.column_name == "id") {
					step_id = e.column_value;
				}
				else {
					seconds = e.column_value;
				}
			}
		);
		ASSERT_STRNEQ2("bogus", step_id);
		ASSERT_STREQ2("200", seconds);
	}

	// Localised information is saved for the created step.
	{
		std::string description;
		stmt(
			fmt::format(
				"SELECT description "
				"FROM recipe_step_localisation "
				"WHERE recipe_step_id = {} AND locale_id = 2",
				step_id
			),
			[&] (auto e) {
				description = e.column_value;
			}
		);
		ASSERT_STREQ2("1 - make cheese; 2 - eat cheese;", description);
	}

	// Should not save if recipe id is not a valid int.
	{
		std::string prev_count_rs;
		stmt(
			"SELECT count(id) FROM recipe_step",
			[&] (auto e) {
				prev_count_rs = e.column_value;
			}
		);
		std::string prev_count_rsl;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				prev_count_rsl = e.column_value;
			}
		);

		wholth::add_steps("", steps, db_con, "2");

		std::string count_rs;
		stmt(
			"SELECT count(id) FROM recipe_step",
			[&] (auto e) {
				count_rs = e.column_value;
			}
		);
		std::string count_rsl;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				count_rsl = e.column_value;
			}
		);

		ASSERT_STREQ3(prev_count_rs, count_rs);
		ASSERT_STREQ3(prev_count_rsl, count_rsl);
	}

	// Should not save if recipe id does not exist.
	{
		std::string prev_count_rs;
		stmt(
			"SELECT count(id) FROM recipe_step",
			[&] (auto e) {
				prev_count_rs = e.column_value;
			}
		);
		std::string prev_count_rsl;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				prev_count_rsl = e.column_value;
			}
		);

		wholth::add_steps("444", steps, db_con, "2");

		std::string count_rs;
		stmt(
			"SELECT count(id) FROM recipe_step",
			[&] (auto e) {
				count_rs = e.column_value;
			}
		);
		std::string count_rsl;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				count_rsl = e.column_value;
			}
		);

		ASSERT_STREQ3(prev_count_rs, count_rs);
		ASSERT_STREQ3(prev_count_rsl, count_rsl);
	}

	// Should not save if locale id is not a valid int.
	{
		std::string prev_count_rs;
		stmt(
			"SELECT count(id) FROM recipe_step",
			[&] (auto e) {
				prev_count_rs = e.column_value;
			}
		);
		std::string prev_count_rsl;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				prev_count_rsl = e.column_value;
			}
		);

		wholth::add_steps("1", steps, db_con, "");

		std::string count_rs;
		stmt(
			"SELECT count(id) FROM recipe_step",
			[&] (auto e) {
				count_rs = e.column_value;
			}
		);
		std::string count_rsl;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				count_rsl = e.column_value;
			}
		);

		ASSERT_STREQ3(prev_count_rs, count_rs);
		ASSERT_STREQ3(prev_count_rsl, count_rsl);
	}

	// Should not save if locale id does not exist.
	{
		std::string prev_count_rs;
		stmt(
			"SELECT count(id) FROM recipe_step",
			[&] (auto e) {
				prev_count_rs = e.column_value;
			}
		);
		std::string prev_count_rsl;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				prev_count_rsl = e.column_value;
			}
		);

		wholth::add_steps("1", steps, db_con, "800");

		std::string count_rs;
		stmt(
			"SELECT count(id) FROM recipe_step",
			[&] (auto e) {
				count_rs = e.column_value;
			}
		);
		std::string count_rsl;
		stmt(
			"SELECT count(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				count_rsl = e.column_value;
			}
		);

		ASSERT_STREQ3(prev_count_rs, count_rs);
		ASSERT_STREQ3(prev_count_rsl, count_rsl);
	}
}

TEST_F(MigrationAwareTest, remove_steps)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'RU'),(2,'FR')");
	stmt(
		"INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date'),"
		"(2,'a date'),"
		"(3,'a date'),"
		"(4,'a date')"
	);
	stmt(
		"INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
		"(10,1,120),"
		"(11,1,120),"
		"(20,2,60)"
	);
	stmt(
		"INSERT INTO recipe_step_localisation (recipe_step_id,locale_id,description) VALUES "
		"(10,1,'desc of recipe 10'),"
		"(20,2,'desc of recipe 20')"
	);
	stmt(
		"INSERT INTO recipe_step_food (recipe_step_id,food_id) "
		"VALUES "
		"(10,3),"
		"(10,4),"
		"(20,4)"
	);

	std::vector<wholth::entity::editable::food::RecipeStep> steps {{
		{.id="10"},
	}};
	wholth::remove_steps(steps, db_con);

	std::string id {"bogus"};
	// Steps are deleted.
	{
		stmt(
			"SELECT id FROM recipe_step WHERE id = 10",
			[&] (auto e) {
				id = e.column_value;
			}
		);
		ASSERT_STREQ2("bogus", id);
	}

	// Speciefied step localisation is deleted.
	{
		std::string loc_count {"bogus"};
		stmt(
			"SELECT COUNT(description) FROM recipe_step_localisation WHERE recipe_step_id = 10",
			[&] (auto e) {
				loc_count = e.column_value;
			}
		);
		ASSERT_STREQ2("0", loc_count);
	}

	// Localisations for steps of other recipes is intact.
	{
		std::string total_loc_count {"bogus"};
		stmt(
			"SELECT COUNT(recipe_step_id) FROM recipe_step_localisation",
			[&] (auto e) {
				total_loc_count = e.column_value;
			}
		);
		ASSERT_STREQ2("1", total_loc_count);
	}

	// Ingredients for scepcified steps are deleted.
	{
		std::string food_count {"bogus"};
		stmt(
			"SELECT COUNT(recipe_step_id) FROM recipe_step_food WHERE recipe_step_id = 10",
			[&] (auto e) {
				food_count = e.column_value;
			}
		);
		ASSERT_STREQ2("0", food_count);
	}

	// Ingredients of other steps are intact.
	{
		std::string total_food_count {"bogus"};
		stmt(
			"SELECT COUNT(recipe_step_id) FROM recipe_step_food",
			[&] (auto e) {
				total_food_count = e.column_value;
			}
		);
		ASSERT_STREQ2("1", total_food_count);
	}
}

TEST_F(MigrationAwareTest, add_nutrients)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'RU')");
	stmt(
		"INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date'),"
		"(2,'a date'),"
		"(3,'a date'),"
		"(4,'a date')"
	);
	stmt(
		"INSERT INTO nutrient (id,unit) "
		"VALUES "
		"(1,'mg'),"
		"(2,'ml'),"
		"(3,'mg')"
	);

	std::vector<wholth::entity::editable::food::Nutrient> nutrients {
		{.id="1",.value="100"},
		{.id="2",.value="40.7"},
		{.id="3",.value="0.0023"},
	};

	wholth::add_nutrients("2", nutrients, db_con);

	// Nutrient info is added for specified food.
	{
		std::vector<std::string> _nutrients;
		stmt(
			"SELECT nutrient_id, value FROM food_nutrient WHERE food_id = 2 ORDER BY nutrient_id",
			[&](auto e) {
				_nutrients.emplace_back(e.column_value);
			}
		);
		ASSERT_EQ(6, _nutrients.size());
		ASSERT_STREQ2("1", _nutrients[0]);
		ASSERT_STREQ2("100", _nutrients[1]);
		ASSERT_STREQ2("2", _nutrients[2]);
		ASSERT_STREQ2("40.7", _nutrients[3]);
		ASSERT_STREQ2("3", _nutrients[4]);
		ASSERT_STREQ2("0.0023", _nutrients[5]);
	}
}

TEST_F(MigrationAwareTest, update_nutrients)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'RU')");
	stmt(
		"INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date'),"
		"(2,'a date'),"
		"(3,'a date'),"
		"(4,'a date')"
	);
	stmt(
		"INSERT INTO nutrient (id,unit) "
		"VALUES "
		"(1,'mg'),"
		"(2,'ml'),"
		"(3,'mg')"
	);
	stmt(
		"INSERT INTO food_nutrient (food_id, nutrient_id, value) "
		"VALUES "
		"(3,1,20),"
		"(2,1,10),"
		"(3,3,0.89),"
		"(1,2,1.6)"
	);

	std::vector<wholth::entity::editable::food::Nutrient> nutrients {
		{.id="1",.value="20.31"},
		{.id="3",.value="0.88"},
	};

	wholth::update_nutrients("3", nutrients, db_con);

	// Nutrient info is updated for specified food.
	{
		std::vector<std::string> _nutrients;
		stmt(
			"SELECT nutrient_id, value FROM food_nutrient WHERE food_id = 3 ORDER BY nutrient_id",
			[&](auto e) {
				_nutrients.emplace_back(e.column_value);
			}
		);
		ASSERT_EQ(4, _nutrients.size());
		ASSERT_STREQ2("1", _nutrients[0]);
		ASSERT_STREQ2("20.31", _nutrients[1]);
		ASSERT_STREQ2("3", _nutrients[2]);
		ASSERT_STREQ2("0.88", _nutrients[3]);
	}
}

TEST_F(MigrationAwareTest, remove_nutrients)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'RU')");
	stmt(
		"INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date'),"
		"(2,'a date'),"
		"(3,'a date'),"
		"(4,'a date')"
	);
	stmt(
		"INSERT INTO nutrient (id,unit) "
		"VALUES "
		"(1,'mg'),"
		"(2,'ml'),"
		"(3,'mg')"
	);
	stmt(
		"INSERT INTO food_nutrient (food_id, nutrient_id, value) "
		"VALUES "
		"(3,1,20),"
		"(2,1,10),"
		"(3,3,0.89),"
		"(1,2,1.6)"
	);

	std::vector<wholth::entity::editable::food::Nutrient> nutrients {
		{.id="3"},
	};

	wholth::remove_nutrients("3", nutrients, db_con);

	// Nutrient info is deleted for specified food.
	{
		std::string value;
		std::size_t iters = 0;
		stmt(
			"SELECT value FROM food_nutrient WHERE food_id = 3",
			[&](auto e) {
				value = e.column_value;
				iters++;
			}
		);
		ASSERT_STREQ2("20", value);
		ASSERT_EQ(1, iters);
	}

	// Nutrient info for other foods is not deleted.
	{
		std::string n_count;
		stmt(
			"SELECT COUNT(nutrient_id) FROM food_nutrient",
			[&](auto e) {
				n_count = e.column_value;
			}
		);
		ASSERT_STREQ2("3", n_count);
	}
}

TEST_F(MigrationAwareTest, add_ingredients)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'RU')");
	stmt(
		"INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date'),"
		"(2,'a date'),"
		"(3,'a date'),"
		"(4,'a date')"
	);
	stmt(
		"INSERT INTO recipe_step (id,recipe_id,seconds) "
		"VALUES "
		"(1,1,40),"
		"(2,3,10)"
	);

	std::vector<wholth::entity::editable::food::Ingredient> ingrs {
		{.food_id="3", .canonical_mass="100"},
		{.food_id="4", .canonical_mass="45"},
	};

	wholth::add_ingredients("2", ingrs, db_con);
	
	// Ingredients are added to the specified step.
	{
		std::vector<std::string> _ings;
		stmt(
			"SELECT food_id, canonical_mass "
			"FROM recipe_step_food "
			"WHERE recipe_step_id = 2 "
			"ORDER BY food_id ASC",
			[&] (auto e) {
				_ings.emplace_back(e.column_value);
			}
		);
		ASSERT_EQ(4, _ings.size());
		ASSERT_STREQ2("3", _ings[0]);
		ASSERT_STREQ2("100", _ings[1]);
		ASSERT_STREQ2("4", _ings[2]);
		ASSERT_STREQ2("45", _ings[3]);
	}
}

TEST_F(MigrationAwareTest, update_ingredients)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'RU')");
	stmt(
		"INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date'),"
		"(2,'a date'),"
		"(3,'a date'),"
		"(4,'a date')"
	);
	stmt(
		"INSERT INTO recipe_step (id,recipe_id,seconds) "
		"VALUES "
		"(100,1,40),"
		"(200,3,10)"
	);
	// 1
	// - 100
	// - 2
	// - 3
	// -- 200
	// -- 4
	stmt(
		"INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) "
		"VALUES "
		"(100,2,70),"
		"(100,3,100),"
		"(200,4,90)"
	);

	std::vector<wholth::entity::editable::food::Ingredient> ingrs {
		{.food_id="2", .canonical_mass="75"},
		{.food_id="3", .canonical_mass="96"},
	};

	// Ingredient masses are updated from specified step.
	{
		wholth::update_ingredients("100", ingrs, db_con);
	
		std::vector<std::string> _ings;
		stmt(
			"SELECT food_id, canonical_mass "
			"FROM recipe_step_food "
			"WHERE recipe_step_id = 100 "
			"ORDER BY food_id ASC",
			[&] (sqlw::Statement::ExecArgs e) {
				_ings.emplace_back(e.column_value);
			}
		);
		ASSERT_EQ(4, _ings.size());
		ASSERT_STREQ2("2", _ings[0]);
		ASSERT_STREQ2("75", _ings[1]);
		ASSERT_STREQ2("3", _ings[2]);
		ASSERT_STREQ2("96", _ings[3]);
	}
}

TEST_F(MigrationAwareTest, recalc_nutrients)
{
	sqlw::Statement stmt {&db_con};

	stmt(
		"INSERT INTO locale (id,alias) VALUES "
		"(1,'EN'),(2,'RU'),(3,'DE')"
	);
	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'10-10-2010'), "
		"(2,'10-10-2010'), "
		"(3,'10-10-2010'),"
		"(4,'10-10-2010'), "
		"(5,'10-10-2010'), "
		"(6,'10-10-2010'), "
		"(7,'10-10-2010'), "
		"(8,'10-10-2010'), "
		"(9,'10-10-2010'), "
		"(10,'10-10-2010'), "
		"(11,'10-10-2010')"
	);
	// 1
	// - 100
	// - 2
	// - 200
	// - 3
	// -- 300
	// -- 9
	// -- 10
	// - 4
	// -- 400
	// -- 5
	// --- 500
	// --- 6
	// ---- 600
	// ---- 7
	// ---- 8
	// --- 700
	// --- 9
	// 11
	// - 800
	// - 6
	stmt("INSERT INTO recipe_step (id,recipe_id) VALUES "
		"(100, 1), "
		"(200, 1), "
		"(300, 3), "
		"(400, 4), "
		"(500, 5), "
		"(600, 6), "
		"(700, 5), "
		"(800, 11)"
	);
	stmt("INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) VALUES "
		"(100, 2, 21), "
		"(200, 3, 31), "
		"(300, 10, 103), "
		"(300, 9, 93), "
		"(200, 4, 41), "
		"(400, 5, 54), "
		"(500, 6, 65), "
		"(600, 7, 76), "
		"(600, 8, 86), "
		"(700, 9, 95), "
		"(800, 6, 60)"
	);
	stmt("INSERT INTO nutrient (id, unit, position) "
		"VALUES "
		"(1,'mg1',3), "
		"(2,'mg2',1), "
		"(3,'mg3',2), "
		"(4,'mg4',0), "
		"(5,'mg5',4) "
	);
	stmt("INSERT INTO nutrient_localisation (nutrient_id, locale_id, title) "
		"VALUES "
		"(1, 1, 'calories'), "
		"(2, 1, 'proteins'), "
		"(3, 1, 'fats'), "
		"(4, 1, 'sugars'), "
		"(5, 1, 'whatevers') "
	);
	stmt("INSERT INTO food_nutrient (food_id, nutrient_id, value) "
		"VALUES "
		"(1, 1, 110),"
		"(1, 2, 120),"
		"(1, 3, 130),"
		"(1, 4, 140),"
		"(1, 5, 150),"
		"(2, 1, 210.5), "
		/* "(2, 2, 220), " */
		"(2, 3, 230), "
		"(2, 4, 240), "
		"(2, 5, 250), "
		"(3, 1, 310), "
		"(3, 2, 320), "
		"(3, 3, 330), "
		"(3, 4, 340), "
		"(3, 5, 350), "
		"(5, 1, 510), "
		"(5, 2, 520), "
		"(5, 3, 530), "
		"(5, 4, 540), "
		"(5, 5, 550), "
		"(7, 1, 710), "
		"(7, 2, 720), "
		"(7, 3, 730), "
		"(7, 4, 740), "
		"(7, 5, 750), "
		"(8, 1, 810), "
		"(8, 2, 820), "
		"(8, 3, 830), "
		"(8, 4, 840), "
		"(8, 5, 850), "
		"(9, 1, 910), "
		"(9, 2, 920), "
		"(9, 3, 930), "
		"(9, 4, 940), "
		"(9, 5, 950), "
		"(10, 1, 1010), "
		"(10, 2, 1020), "
		"(10, 3, 1030), "
		"(10, 4, 1040), "
		"(10, 5, 1050), "
		"(11, 1, 1110), "
		"(11, 2, 1120), "
		"(11, 3, 1130), "
		"(11, 4, 1140), "
		"(11, 5, 1150)"
	);

	constexpr size_t str_len = 4;

	// todo test case when updateing ingredients that is not part of recipe.
	{
		wholth::recalc_nutrients("1", db_con);

		std::vector<std::string> expected_values {
			// 4
			std::to_string(100.0/(21+31+41) * (
				21.0/(21+31+41) * 240 + // 2
				31.0/(21+31+41) * ( // 3
					103.0/(103+93) * 1040 + // 10
					93.0/(103+93) * 940	   // 9
				) +
				41.0/(21+31+41) * ( // 4
					54.0/54 * ( // 5
						65.0/(65+95) * // 6
						(
							76.0/(76+86) * 740 + // 7
							86.0/(76+86) * 840   // 8
						) +
						95.0/(65+95) * 940 // 9
					)
				)
			)).substr(0, str_len),
			// 2
			"120",
			// 3
			std::to_string(100.0/(21+31+41) * (
				21.0/(21+31+41) * 230 + // 2
				31.0/(21+31+41) * ( // 3
					103.0/(103+93) * 1030 + // 10
					93.0/(103+93) * 930	   // 9
				) +
				41.0/(21+31+41) * ( // 4
					54.0/54 * ( // 5
						65.0/(65+95) * // 6
						(
							76.0/(76+86) * 730 + // 7
							86.0/(76+86) * 830   // 8
						) +
						95.0/(65+95) * 930 // 9
					)
				)
			)).substr(0, str_len),
			// 1
			std::to_string(100.0/(21+31+41) * (
				21.0/(21+31+41) * 210 + // 2
				31.0/(21+31+41) * ( // 3
					103.0/(103+93) * 1010 + // 10
					93.0/(103+93) * 910	   // 9
				) +
				41.0/(21+31+41) * ( // 4
					54.0/54 * ( // 5
						65.0/(65+95) * // 6
						(
							76.0/(76+86) * 710 + // 7
							86.0/(76+86) * 810   // 8
						) +
						95.0/(65+95) * 910 // 9
					)
				)
			)).substr(0, str_len),
			// 5
			"150",
		};
		std::vector<std::string> values {};
		stmt(
			"SELECT fn.value "
			"FROM food_nutrient fn "
			"INNER JOIN nutrient n "
				"ON n.id = fn.nutrient_id "
			"WHERE fn.food_id = 1 "
			"ORDER BY n.position ASC ",
			[&] (sqlw::Statement::ExecArgs e) {
				/* fmt::print("{}: {}\n", e.column_name, e.column_value); */
				values.emplace_back(e.column_value);
			}
		);

		ASSERT_EQ(expected_values.size(), values.size());
		ASSERT_STREQ3(expected_values[0], values[0].substr(0, str_len));
		ASSERT_STREQ3(expected_values[1], values[1].substr(0, str_len));
		ASSERT_STREQ3(expected_values[2], values[2].substr(0, str_len));
		ASSERT_STREQ3(expected_values[3], values[3].substr(0, str_len));
		ASSERT_STREQ3(expected_values[4], values[4].substr(0, str_len));
	}
}

TEST_F(MigrationAwareTest, remove_ingredients)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'RU')");
	stmt(
		"INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'a date'),"
		"(2,'a date'),"
		"(3,'a date'),"
		"(4,'a date')"
	);
	stmt(
		"INSERT INTO recipe_step (id,recipe_id,seconds) "
		"VALUES "
		"(1,1,40),"
		"(2,3,10)"
	);
	stmt(
		"INSERT INTO recipe_step_localisation (recipe_step_id,locale_id,description) "
		"VALUES "
		"(1,1,'Step 1 for food 2'),"
		"(2,1,'Step 1 for food 3')"
	);
	stmt(
		"INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) "
		"VALUES "
		"(1,2,70),"
		"(1,3,100),"
		"(2,3,90),"
		"(2,4,90)"
	);

	std::vector<wholth::entity::editable::food::Ingredient> ingrs {
		{.food_id="2"},
		{.food_id="3"},
	};

	wholth::remove_ingredients("2", ingrs, db_con);
	
	// Ingredients are removed from specified step.
	{
		std::string count;
		stmt(
			"SELECT COUNT(food_id) "
			"FROM recipe_step_food "
			"WHERE recipe_step_id = 2",
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("1", count);
	}

	// Ingredients are not removed from other steps.
	{
		std::string count;
		stmt(
			"SELECT COUNT(food_id) "
			"FROM recipe_step_food",
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("3", count);
	}
}

TEST_F(MigrationAwareTest, expand_food)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		" (1,'10-10-2010'),"
		" (2,'10-10-2010')"
	);
	stmt(
		"INSERT INTO locale (id,alias) VALUES "
		"(1,'EN'),(2,'RU'),(3,'DE')"
	);
	stmt("INSERT INTO food_localisation (food_id, locale_id, title, description) "
		"VALUES "
		"(1, 1, 'salt', 'Essence of salt'),"
		"(2, 2, 'cacao', NULL),"
		"(2, 3, 'cacoka', 'A cacaoky thing');"
	);
	stmt("INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
		"(1, 2, 600) "
	);
	stmt("INSERT INTO recipe_step_localisation (recipe_step_id,locale_id,description) VALUES "
		"(1, 2, 'do some mixing') "
	);

	{
		wholth::entity::expanded::Food food;
		std::string buffer;

		auto rc = wholth::expand_food(
			food,
			buffer,
			"2",
			"3",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("2", food.id);
		ASSERT_STREQ2("cacoka", food.title);
		ASSERT_STREQ2("A cacaoky thing", food.description);
		ASSERT_STREQ2("10m", food.preparation_time);
	}

	{
		wholth::entity::expanded::Food food;
		std::string buffer;

		auto rc = wholth::expand_food(
			food,
			buffer,
			"2",
			"1",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("2", food.id);
		ASSERT_STREQ2("[N/A]", food.title);
		ASSERT_STREQ2("[N/A]", food.description);
		ASSERT_STREQ2("10m", food.preparation_time);
	}

	{
		wholth::entity::expanded::Food food;
		std::string buffer;

		auto rc = wholth::expand_food(
			food,
			buffer,
			"1",
			"1",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("1", food.id);
		ASSERT_STREQ2("salt", food.title);
		ASSERT_STREQ2("Essence of salt", food.description);
		ASSERT_STREQ2("[N/A]", food.preparation_time);
	}

	// No food found.
	{
		wholth::entity::expanded::Food food;
		std::string buffer;

		auto rc = wholth::expand_food(
			food,
			buffer,
			"3",
			"1",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::ENTITY_NOT_FOUND, rc) << wholth::view(rc);
		ASSERT_STREQ2("", food.id);
		ASSERT_STREQ2("", food.title);
		ASSERT_STREQ2("", food.description);
		ASSERT_STREQ2("", food.preparation_time);
	}

	// SQL statement error.
	{
		wholth::entity::expanded::Food food;
		std::string buffer;

		auto rc = wholth::expand_food(
			food,
			buffer,
			"1",
			"",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc) << wholth::view(rc);
		ASSERT_STREQ2("", food.id);
		ASSERT_STREQ2("", food.title);
		ASSERT_STREQ2("", food.description);
		ASSERT_STREQ2("", food.preparation_time);
	}
}

TEST_F(MigrationAwareTest, list_steps)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		" (1,'10-10-2010'),"
		" (2,'10-10-2010')"
	);
	stmt(
		"INSERT INTO locale (id,alias) VALUES "
		"(1,'EN'),(2,'RU'),(3,'DE')"
	);
	stmt("INSERT INTO food_localisation (food_id, locale_id, title, description) "
		"VALUES "
		"(1, 1, 'salt', 'Essence of salt'), "
		"(2, 2, 'cacao', NULL), "
		"(2, 3, 'cacoka', 'A cacaoky thing')"
	);
	stmt("INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
		"(1, 2, '3660'), "
		"(2, 2, '60'), "
		"(3, 1, '100'), "
		"(4, 2, '120') "
	);
	stmt("INSERT INTO recipe_step_localisation (recipe_step_id,locale_id,description) VALUES "
		"(1, 2, 'do some mixing'), "
		"(2, 2, 'do another mixing') "
	);

	{
		std::array<wholth::entity::expanded::food::RecipeStep, 1> steps {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_steps(
			steps,
			buffer,
			"2",
			"2",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("1", steps[0].id);
		ASSERT_STREQ2("do some mixing", steps[0].description);
		ASSERT_STREQ2("1h 1m", steps[0].time);
	}

	{
		std::array<wholth::entity::expanded::food::RecipeStep, 1> steps {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_steps(
			steps,
			buffer,
			"2",
			"",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc) << wholth::view(rc);
		ASSERT_STREQ2("", steps[0].id);
		ASSERT_STREQ2("", steps[0].description);
		ASSERT_STREQ2("", steps[0].time);
	}

	{
		std::array<wholth::entity::expanded::food::RecipeStep, 1> steps {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_steps(
			steps,
			buffer,
			"2",
			"2abob",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc) << wholth::view(rc);
		ASSERT_STREQ2("", steps[0].id);
		ASSERT_STREQ2("", steps[0].description);
		ASSERT_STREQ2("", steps[0].time);
	}

	{
		std::array<wholth::entity::expanded::food::RecipeStep, 1> steps {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_steps(
			steps,
			buffer,
			"33",
			"2",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::ENTITY_NOT_FOUND, rc) << wholth::view(rc);
		ASSERT_STREQ2("", steps[0].id);
		ASSERT_STREQ2("", steps[0].description);
		ASSERT_STREQ2("", steps[0].time);
	}

	{
		std::array<wholth::entity::expanded::food::RecipeStep, 3> steps {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_steps(
			steps,
			buffer,
			"2",
			"2",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("1", steps[0].id);
		ASSERT_STREQ2("do some mixing", steps[0].description);
		ASSERT_STREQ2("1h 1m", steps[0].time);
		ASSERT_STREQ2("2", steps[1].id);
		ASSERT_STREQ2("do another mixing", steps[1].description);
		ASSERT_STREQ2("1m", steps[1].time);
		ASSERT_STREQ2("4", steps[2].id);
		ASSERT_STREQ2("[N/A]", steps[2].description);
		ASSERT_STREQ2("2m", steps[2].time);
	}

	{
		std::array<wholth::entity::expanded::food::RecipeStep, 2> steps {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_steps(
			steps,
			buffer,
			"1",
			"2",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("3", steps[0].id);
		ASSERT_STREQ2("[N/A]", steps[0].description);
		ASSERT_STREQ2("1m", steps[0].time);
		ASSERT_STREQ2("", steps[1].id);
		ASSERT_STREQ2("", steps[1].description);
		ASSERT_STREQ2("", steps[1].time);
	}
}

TEST_F(MigrationAwareTest, list_ingredients)
{
	sqlw::Statement stmt {&db_con};

	stmt(
		"INSERT INTO locale (id,alias) VALUES "
		"(1,'EN'),(2,'RU'),(3,'DE')"
	);
	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'10-10-2010'), "
		"(2,'10-10-2010'), "
		"(3,'10-10-2010'),"
		"(4,'10-10-2010'),"
		"(5,'10-10-2010')"
	);
	stmt("INSERT INTO food_localisation (food_id, locale_id, title, description) "
		"VALUES "
		"(1, 1, 'salt', 'Essence of salt'), "
		"(2, 2, 'cacao', NULL), "
		"(2, 3, 'cacoka', 'A cacaoky thing'), "
		"(3, 2, 'water', 'A watery thing'), "
		"(4, 2, 'metal', 'A metaly thing'), "
		"(5, 2, 'meat', 'A meaty thing')"
	);
	stmt("INSERT INTO recipe_step (id,recipe_id) VALUES "
		"(1, 1), "
		"(2, 1), "
		"(3, 4)"
	);
	stmt("INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) VALUES "
		"(1, 2, 433.4), "
		"(2, 3, 100), "
		"(2, 4, 50), "
		"(3, 5, 100)"
	);

	{
		std::array<wholth::entity::expanded::food::Ingredient, 1> ingrs {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_ingredients(
			ingrs,
			buffer,
			"1",
			"2",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("2", ingrs[0].food_id);
		ASSERT_STREQ2("cacao", ingrs[0].title);
		ASSERT_STREQ2("433.4", ingrs[0].canonical_mass);
		ASSERT_STREQ2("0", ingrs[0].ingredient_count);
	}

	{
		std::array<wholth::entity::expanded::food::Ingredient, 4> ingrs {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_ingredients(
			ingrs,
			buffer,
			"1",
			"2",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("2", ingrs[0].food_id);
		ASSERT_STREQ2("cacao", ingrs[0].title);
		ASSERT_STREQ2("433.4", ingrs[0].canonical_mass);
		ASSERT_STREQ2("0", ingrs[0].ingredient_count);
		ASSERT_STREQ2("3", ingrs[1].food_id);
		ASSERT_STREQ2("water", ingrs[1].title);
		ASSERT_STREQ2("100", ingrs[1].canonical_mass);
		ASSERT_STREQ2("0", ingrs[1].ingredient_count);
		ASSERT_STREQ2("4", ingrs[2].food_id);
		ASSERT_STREQ2("metal", ingrs[2].title);
		ASSERT_STREQ2("50", ingrs[2].canonical_mass);
		ASSERT_STREQ2("1", ingrs[2].ingredient_count);
		ASSERT_STREQ2("", ingrs[3].food_id);
		ASSERT_STREQ2("", ingrs[3].title);
		ASSERT_STREQ2("", ingrs[3].canonical_mass);
		ASSERT_STREQ2("", ingrs[3].ingredient_count);
	}

	{
		std::array<wholth::entity::expanded::food::Ingredient, 2> ingrs {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_ingredients(
			ingrs,
			buffer,
			"1",
			"3",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("2", ingrs[0].food_id);
		ASSERT_STREQ2("cacoka", ingrs[0].title);
		ASSERT_STREQ2("433.4", ingrs[0].canonical_mass);
		ASSERT_STREQ2("0", ingrs[0].ingredient_count);
		ASSERT_STREQ2("3", ingrs[1].food_id);
		ASSERT_STREQ2("[N/A]", ingrs[1].title);
		ASSERT_STREQ2("100", ingrs[1].canonical_mass);
		ASSERT_STREQ2("0", ingrs[1].ingredient_count);
	}

	{
		std::array<wholth::entity::expanded::food::Ingredient, 2> ingrs {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_ingredients(
			ingrs,
			buffer,
			"1",
			"",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc) << wholth::view(rc);
	}

	{
		std::array<wholth::entity::expanded::food::Ingredient, 2> ingrs {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_ingredients(
			ingrs,
			buffer,
			"1",
			"3a",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc) << wholth::view(rc);
	}

	{
		std::array<wholth::entity::expanded::food::Ingredient, 2> ingrs {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_ingredients(
			ingrs,
			buffer,
			"10",
			"2",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::ENTITY_NOT_FOUND, rc) << wholth::view(rc);
	}

	{
		std::array<wholth::entity::expanded::food::Ingredient, 2> ingrs {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_ingredients(
			ingrs,
			buffer,
			"",
			"2",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::ENTITY_NOT_FOUND, rc) << wholth::view(rc);
	}
}

TEST_F(MigrationAwareTest, list_nutrients)
{
	sqlw::Statement stmt {&db_con};

	stmt(
		"INSERT INTO locale (id,alias) VALUES "
		"(1,'EN'),(2,'RU'),(3,'DE')"
	);
	stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		"(1,'10-10-2010'), "
		"(2,'10-10-2010'), "
		"(3,'10-10-2010'),"
		"(4,'10-10-2010'), "
		"(5,'10-10-2010'), "
		"(6,'10-10-2010'), "
		"(7,'10-10-2010'), "
		"(8,'10-10-2010'), "
		"(9,'10-10-2010'), "
		"(10,'10-10-2010'), "
		"(11,'10-10-2010')"
	);
	stmt("INSERT INTO recipe_step (id,recipe_id) VALUES "
		"(100, 1), "
		"(200, 1), "
		"(300, 3), "
		"(400, 4), "
		"(500, 5), "
		"(600, 6), "
		"(700, 5), "
		"(800, 11)"
	);
	stmt("INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) VALUES "
		"(100, 2, 21), "
		"(200, 3, 31), "
		"(300, 10, 103), "
		"(300, 9, 93), "
		"(200, 4, 41), "
		"(400, 5, 54), "
		"(500, 6, 65), "
		"(600, 7, 76), "
		"(600, 8, 86), "
		"(700, 9, 95), "
		"(800, 6, 60)"
	);
	stmt("INSERT INTO nutrient (id, unit, position) "
		"VALUES "
		"(1,'mg1',3), "
		"(2,'mg2',1), "
		"(3,'mg3',2) "
	);
	stmt("INSERT INTO nutrient_localisation (nutrient_id, locale_id, title) "
		"VALUES "
		"(1, 1, 'calories'), "
		"(2, 1, 'proteins'), "
		"(3, 1, 'fats') "
	);
	stmt("INSERT INTO food_nutrient (food_id, nutrient_id, value) "
		"VALUES "
		"(2, 1, 210.5), "
		/* "(2, 2, 220), " */
		"(2, 3, 230), "
		"(3, 1, 310), "
		"(3, 2, 320), "
		"(3, 3, 330), "
		"(5, 1, 510), "
		"(5, 2, 520), "
		"(5, 3, 530), "
		"(7, 1, 710), "
		"(7, 2, 720), "
		"(7, 3, 730), "
		"(8, 1, 810), "
		"(8, 2, 820), "
		"(8, 3, 830), "
		"(9, 1, 910), "
		"(9, 2, 920), "
		"(9, 3, 930), "
		"(10, 1, 1010), "
		"(10, 2, 1020), "
		"(10, 3, 1030), "
		"(11, 1, 1110), "
		"(11, 2, 1120), "
		"(11, 3, 1130) "
	);

	{
		std::array<wholth::entity::expanded::food::Nutrient, 1> nuts {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_nutrients(
			nuts,
			buffer,
			"1",
			"1",
			&db_con
		);
		auto expected_fats = std::to_string(
			(
				21.0/(21+31+41) * 230 + // 2
				31.0/(21+31+41) * ( // 3
					103.0/(103+93) * 1030 + // 10
					93.0/(103+93) * 930	   // 9
				) +
				41.0/(21+31+41) * ( // 4
					54.0/54 * ( // 5
						65.0/(65+95) * // 6
						(
							76.0/(76+86) * 730 + // 7
							86.0/(76+86) * 830   // 8
						) +
						95.0/(65+95) * 930 // 9
					)
				)
			) * 100/(21+31+41)
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("3", nuts[0].id);
		ASSERT_STREQ2("fats", nuts[0].title);
		ASSERT_STREQ3(expected_fats.substr(0, 6), nuts[0].value.substr(0, 6));
		ASSERT_STREQ2("mg3", nuts[0].unit);
		ASSERT_STREQ2("", nuts[0].user_value);
	}

	{
		std::array<wholth::entity::expanded::food::Nutrient, 1> nuts {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_nutrients(
			nuts,
			buffer,
			"1",
			"3",
			&db_con
		);
		auto expected_fats = std::to_string(
			(
				21.0/(21+31+41) * 230 + // 2
				31.0/(21+31+41) * ( // 3
					103.0/(103+93) * 1030 + // 10
					93.0/(103+93) * 930	   // 9
				) +
				41.0/(21+31+41) * ( // 4
					54.0/54 * ( // 5
						65.0/(65+95) * // 6
						(
							76.0/(76+86) * 730 + // 7
							86.0/(76+86) * 830   // 8
						) +
						95.0/(65+95) * 930 // 9
					)
				)
			) * 100/(21+31+41)
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("3", nuts[0].id);
		ASSERT_STREQ2("[N/A]", nuts[0].title);
		ASSERT_STREQ3(expected_fats.substr(0, 6), nuts[0].value.substr(0, 6));
		ASSERT_STREQ2("mg3", nuts[0].unit);
		ASSERT_STREQ2("", nuts[0].user_value);
	}

	{
		std::array<wholth::entity::expanded::food::Nutrient, 4> nuts {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_nutrients(
			nuts,
			buffer,
			"11",
			"1",
			&db_con
		);
		auto expected_calories = std::to_string(
			60.0/60 * ( // 6
				76.0/(76+86) * 710 + // 7
				86.0/(76+86) * 810   // 8
			) * 100/60
		);
		auto expected_proteins = std::to_string(
			60.0/60 * ( // 6
				76.0/(76+86) * 720 + // 7
				86.0/(76+86) * 820   // 8
			) * 100/60
		);
		auto expected_fats = std::to_string(
			60.0/60 * ( // 6
				76.0/(76+86) * 730 + // 7
				86.0/(76+86) * 830   // 8
			) * 100/60
		);

		ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc) << wholth::view(rc);
		ASSERT_STREQ2("2", nuts[0].id);
		ASSERT_STREQ2("proteins", nuts[0].title);
		ASSERT_STREQ3(expected_proteins.substr(0, 6), nuts[0].value.substr(0, 6));
		ASSERT_STREQ2("mg2", nuts[0].unit);
		ASSERT_STREQ2("1120", nuts[0].user_value);
		ASSERT_STREQ2("3", nuts[1].id);
		ASSERT_STREQ2("fats", nuts[1].title);
		ASSERT_STREQ3(expected_fats.substr(0, 6), nuts[1].value.substr(0, 6));
		ASSERT_STREQ2("mg3", nuts[1].unit);
		ASSERT_STREQ2("1130", nuts[1].user_value);
		ASSERT_STREQ2("1", nuts[2].id);
		ASSERT_STREQ2("calories", nuts[2].title);
		ASSERT_STREQ3(expected_calories.substr(0, 6), nuts[2].value.substr(0, 6));
		ASSERT_STREQ2("mg1", nuts[2].unit);
		ASSERT_STREQ2("1110", nuts[2].user_value);
		ASSERT_STREQ2("", nuts[3].id);
		ASSERT_STREQ2("", nuts[3].title);
		ASSERT_STREQ2("", nuts[3].value);
		ASSERT_STREQ2("", nuts[3].unit);
	}

	{
		std::array<wholth::entity::expanded::food::Nutrient, 1> nuts {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_nutrients(
			nuts,
			buffer,
			"11",
			"2a",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc) << wholth::view(rc);
		ASSERT_STREQ2("", nuts[0].id);
		ASSERT_STREQ2("", nuts[0].title);
		ASSERT_STREQ2("", nuts[0].value.substr(0, 6));
		ASSERT_STREQ2("", nuts[0].unit);
		ASSERT_STREQ2("", nuts[0].user_value);
	}

	{
		std::array<wholth::entity::expanded::food::Nutrient, 1> nuts {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_nutrients(
			nuts,
			buffer,
			"11",
			"",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::INVALID_LOCALE_ID, rc) << wholth::view(rc);
		ASSERT_STREQ2("", nuts[0].id);
		ASSERT_STREQ2("", nuts[0].title);
		ASSERT_STREQ2("", nuts[0].value.substr(0, 6));
		ASSERT_STREQ2("", nuts[0].unit);
		ASSERT_STREQ2("", nuts[0].user_value);
	}

	{
		std::array<wholth::entity::expanded::food::Nutrient, 1> nuts {};
		std::string buffer {};
		wholth::StatusCode rc = wholth::list_nutrients(
			nuts,
			buffer,
			"101",
			"2",
			&db_con
		);

		ASSERT_EQ(wholth::StatusCode::ENTITY_NOT_FOUND, rc) << wholth::view(rc);
		ASSERT_STREQ2("", nuts[0].id);
		ASSERT_STREQ2("", nuts[0].title);
		ASSERT_STREQ2("", nuts[0].value.substr(0, 6));
		ASSERT_STREQ2("", nuts[0].unit);
		ASSERT_STREQ2("", nuts[0].user_value);
	}
}
