#include <array>
#include <exception>
#include <gtest/gtest.h>
#include <sstream>
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

TEST_F(MigrationAwareTest, food_query_page)
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
		"(1, 'a', 0),"
		"(2, 'b', 0),"
		"(3, 'c', 0)"
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

	wholth::Pager<wholth::entity::viewable::Food> pager;

	{
		std::array<wholth::entity::viewable::Food, 2> list;
		wholth::FoodsQuery q {
			.page = 0,
			.locale_id = "1",
		};
		PaginationInfo info = pager.query_page(list, &db_con, q);

		ASSERT_STREQ2("1", list[0].id);
		ASSERT_STREQ2("Salt", list[0].title);
		ASSERT_STREQ2("10m", list[0].preparation_time);
		ASSERT_STREQ2("2", list[1].id);
		ASSERT_STREQ2("[N/A]", list[1].title);
		ASSERT_STREQ2("[N/A]", list[1].preparation_time);

		ASSERT_STREQ2("3", info.max_page);
		ASSERT_STREQ2("6", info.element_count);
		ASSERT_STREQ2("1/3", info.progress_string);
	}

	{
		std::array<wholth::entity::viewable::Food, 10> list;
		wholth::FoodsQuery q {
			.page = 0,
			.locale_id = "2",
			.title = "Sal",
		};
		auto info = pager.query_page(list, &db_con, q);

		ASSERT_STREQ2("1", list[0].id);
		ASSERT_STREQ2("Salta", list[0].title);
		ASSERT_STREQ2("10m", list[0].preparation_time);

		ASSERT_STREQ2("4", list[1].id);
		ASSERT_STREQ2("Saltabar", list[1].title);
		ASSERT_STREQ2("[N/A]", list[1].preparation_time);

		ASSERT_STREQ2("5", list[2].id);
		ASSERT_STREQ2("Salia", list[2].title);
		ASSERT_STREQ2("[N/A]", list[2].preparation_time);

		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);
	}

	{
		std::array<wholth::entity::viewable::Food, 10> list;
		wholth::FoodsQuery q {
			.page = 0,
			.locale_id = "",
			.title = "Sal",
		};
		auto info = pager.query_page(list, &db_con, q);

		ASSERT_STREQ2("1", list[0].id);
		ASSERT_STREQ2("Salt", list[0].title);
		ASSERT_STREQ2("10m", list[0].preparation_time);

		ASSERT_STREQ2("4", list[1].id);
		ASSERT_STREQ2("Saltabar", list[1].title);
		ASSERT_STREQ2("[N/A]", list[1].preparation_time);

		ASSERT_STREQ2("5", list[2].id);
		ASSERT_STREQ2("Salia", list[2].title);
		ASSERT_STREQ2("[N/A]", list[2].preparation_time);

		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);
	}

	{
		std::array<wholth::entity::viewable::Food, 2> list;
		wholth::FoodsQuery q {
			.page = 1,
			.locale_id = "2",
			.title = "Sal",
		};
		auto info = pager.query_page(list, &db_con, q);

		// Checks that foods are sorted by id by default.
		ASSERT_STREQ2("5", list[0].id);
		ASSERT_STREQ2("Salia", list[0].title);
		ASSERT_STREQ2("[N/A]", list[0].preparation_time);

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
		std::array<wholth::entity::viewable::Food, 3> list;
		wholth::FoodsQuery q {
			.page = 0,
			.locale_id = "",
			.nutrient_filters = arr,
		};
		PaginationInfo info = pager.query_page(list, &db_con, q);

		ASSERT_STREQ2("3", list[0].id);
		ASSERT_STREQ2("4", list[1].id);
		ASSERT_STREQ2("6", list[2].id);

		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);
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
	wholth::insert_food(food, db_con, "2");

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
				id
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
	wholth::insert_food(food, db_con, "");

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
	wholth::insert_food(food, db_con, "2");

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
		ASSERT_STREQ2("salt", title[1]);
		ASSERT_STREQ2("a food 4 GB", description[0]);
		ASSERT_STREQ2("for nerves", description[1]);
	}

	// Only title is updated for the specified food.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.title="Salti",
		};
		wholth::update_food(food, db_con, "3");

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
	}

	// Only description is updated for the specified food.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.description="Salti is salty",
		};
		wholth::update_food(food, db_con, "3");

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
	}

	// Title is an empty string.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.title="",
		};
		wholth::update_food(food, db_con, "3");

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
	}

	// Title is nothing but spaces.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.title="   ",
		};
		wholth::update_food(food, db_con, "3");

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
	}

	// Description is an empty string.
	{
		wholth::entity::editable::Food food {
			.id="4",
			.description="",
		};
		wholth::update_food(food, db_con, "3");

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
		wholth::remove_food("", db_con);

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
		wholth::remove_food("   ", db_con);

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
		wholth::remove_food("22", db_con);

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
		wholth::remove_food(" 3  ", db_con);

		std::string count = "";
		stmt(
			"SELECT COUNT(id) FROM food",
			[&] (auto e) {
				count = e.column_value;
			}
		);
		ASSERT_STREQ2("3", count);
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
		"(2,3,90)"
	);

	std::vector<wholth::entity::editable::food::Ingredient> ingrs {
		{.food_id="2", .canonical_mass="75"},
		{.food_id="3", .canonical_mass="96"},
	};

	// Ingredient masses are updated from specified step.
	{
		wholth::update_ingredients("1", ingrs, db_con);
	
		std::vector<std::string> _ings;
		stmt(
			"SELECT food_id, canonical_mass "
			"FROM recipe_step_food "
			"WHERE recipe_step_id = 1 "
			"ORDER BY food_id ASC",
			[&] (auto e) {
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
