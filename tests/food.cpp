#include <exception>
#include <gtest/gtest.h>
#include <sstream>
#include <tuple>
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
	using namespace wholth;

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
	stmt("INSERT INTO locale (id,alias) VALUES (1,'EN')");
	stmt("INSERT INTO locale (id,alias) VALUES (2,'RU')");
	stmt("INSERT INTO locale (id,alias) VALUES (3,'DE')");
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
	stmt("INSERT INTO food_nutrient (food_id, nutrient_id, value, created_at) "
		"VALUES "
		"(1, 1, 100, '10-01-2011')," // -
		"(1, 2, 22.4, '10-01-2011'),"
		"(1, 3, 20, '10-01-2011'),"
		"(2, 1, 99, '10-01-2011')," // -
		"(2, 2, 19, '10-01-2011'),"
		"(2, 3, 20, '10-01-2011'),"
		"(3, 1, 100, '10-01-2011')," // +
		"(3, 2, 20, '10-01-2011'),"
		"(3, 3, 10.33, '10-01-2011'),"
		"(4, 1, 100, '10-01-2011')," // +
		"(4, 2, 30, '10-01-2011'),"
		"(4, 3, 39.999, '10-01-2011'),"
		"(5, 1, 10, '10-01-2011')," // -
		"(5, 2, 20, '10-01-2011'),"
		"(5, 3, 20, '10-01-2011'),"
		"(6, 1, 100, '10-01-2011')," // +
		"(6, 2, 0, '10-01-2011'),"
		"(6, 3, 30, '10-01-2011')"
	);
	stmt("INSERT INTO recipe_step (recipe_id,time) VALUES "
		"(1, '600') "
	);

	/* std::array<entity::food::View, 50> list; */
	std::array<wholth::entity::viewable::Food, 50> list;
	/* Pager<entity::food::View> pager; */
	Pager<wholth::entity::viewable::Food> pager;

	{
		FoodsQuery q {
			.limit = 2,
			.page = 0,
			.locale_id = "1",
		};
		PaginationInfo info = pager.query_page(list, &db_con, q);

		/* ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[0])); */
		/* ASSERT_STREQ2("Salt", entity::get<entity::food::view::title>(list[0])); */
		/* ASSERT_STREQ2("Essence of salt", entity::get<entity::food::view::description>(list[0])); */
		/* ASSERT_STREQ2("2", entity::get<entity::food::view::id>(list[1])); */
		/* ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::title>(list[1])); */
		/* ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[1])); */

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
		FoodsQuery q {
			.limit = 10,
			.page = 0,
			.locale_id = "2",
			.title = "Sal",
		};
		auto info = pager.query_page(list, &db_con, q);

		/* ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[0])); */
		/* ASSERT_STREQ2("Salta", entity::get<entity::food::view::title>(list[0])); */
		/* ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[0])); */
		ASSERT_STREQ2("1", list[0].id);
		ASSERT_STREQ2("Salta", list[0].title);
		ASSERT_STREQ2("10m", list[0].preparation_time);

		/* ASSERT_STREQ2("4", entity::get<entity::food::view::id>(list[1])); */
		/* ASSERT_STREQ2("Saltabar", entity::get<entity::food::view::title>(list[1])); */
		/* ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[1])); */
		ASSERT_STREQ2("4", list[1].id);
		ASSERT_STREQ2("Saltabar", list[1].title);
		ASSERT_STREQ2("[N/A]", list[1].preparation_time);

		/* ASSERT_STREQ2("5", entity::get<entity::food::view::id>(list[2])); */
		/* ASSERT_STREQ2("Salia", entity::get<entity::food::view::title>(list[2])); */
		/* ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[2])); */
		ASSERT_STREQ2("5", list[2].id);
		ASSERT_STREQ2("Salia", list[2].title);
		ASSERT_STREQ2("[N/A]", list[2].preparation_time);

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

		/* ASSERT_STREQ2("1", entity::get<entity::food::view::id>(list[0])); */
		/* ASSERT_STREQ2("Salt", entity::get<entity::food::view::title>(list[0])); */
		/* ASSERT_STREQ2("Essence of salt", entity::get<entity::food::view::description>(list[0])); */
		ASSERT_STREQ2("1", list[0].id);
		ASSERT_STREQ2("Salt", list[0].title);
		ASSERT_STREQ2("10m", list[0].preparation_time);

		/* ASSERT_STREQ2("4", entity::get<entity::food::view::id>(list[1])); */
		/* ASSERT_STREQ2("Saltabar", entity::get<entity::food::view::title>(list[1])); */
		/* ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[1])); */
		ASSERT_STREQ2("4", list[1].id);
		ASSERT_STREQ2("Saltabar", list[1].title);
		ASSERT_STREQ2("[N/A]", list[1].preparation_time);

		/* ASSERT_STREQ2("5", entity::get<entity::food::view::id>(list[2])); */
		/* ASSERT_STREQ2("Salia", entity::get<entity::food::view::title>(list[2])); */
		/* ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[2])); */
		ASSERT_STREQ2("5", list[2].id);
		ASSERT_STREQ2("Salia", list[2].title);
		ASSERT_STREQ2("[N/A]", list[2].preparation_time);

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
		/* ASSERT_STREQ2("5", entity::get<entity::food::view::id>(list[0])); */
		/* ASSERT_STREQ2("Salia", entity::get<entity::food::view::title>(list[0])); */
		/* ASSERT_STREQ2("[N/A]", entity::get<entity::food::view::description>(list[0])); */
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
		FoodsQuery q {
			.limit = 3,
			.page = 0,
			.locale_id = "",
			.nutrient_filters = arr,
		};
		PaginationInfo info = pager.query_page(list, &db_con, q);

		/* ASSERT_STREQ2("3", entity::get<entity::food::view::id>(list[0])); */
		/* ASSERT_STREQ2("4", entity::get<entity::food::view::id>(list[1])); */
		/* ASSERT_STREQ2("6", entity::get<entity::food::view::id>(list[2])); */
		ASSERT_STREQ2("3", list[0].id);
		ASSERT_STREQ2("4", list[1].id);
		ASSERT_STREQ2("6", list[2].id);

		ASSERT_STREQ2("1", info.max_page);
		ASSERT_STREQ2("3", info.element_count);
		ASSERT_STREQ2("1/1", info.progress_string);
	}

	// @todo add test cases:
	// - when limit is larger than container size;
}

TEST_F(MigrationAwareTest, food_create)
{
	sqlw::Statement stmt {&db_con};

	stmt("INSERT INTO locale (id,alias) VALUES (1,'EN')");
	stmt("INSERT INTO food (id, created_at) VALUES (1,'a date'),(2,'a date')");
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

	// When nutrients and directions are provided.
	{
		std::vector<wholth::entity::editable::food::Nutrient> nutrients {{
			{.id="1", .value="0.11"},
			{"5", "42.40005"},
			{"2", "33"},
		}};
		std::vector<wholth::entity::editable::food::RecipeStepFood> foods {{
			{.food_id="1",.canonical_mass="350"},
			{"2", "30"},
		}};
		std::vector<wholth::entity::editable::food::RecipeStep> steps {{
			{
				.time="60",
				.description="do fry",
				.foods=foods
			},
		}};
		wholth::entity::editable::Food food {
			.id = "22",
			.title = "Tomato",
			.description = "A red thing",
			.nutrients = nutrients,
			.steps=steps,
		};

		wholth::insert(db_con, food, "1");

		// Should create new row in `food`.
		std::string _id;
		{
			std::string _created_at;
			std::string now = wholth::utils::current_time_and_date();

			stmt(
				"SELECT f.id, f.created_at FROM food f",
				[&] (auto e) {
					if (e.column_name.compare("id") == 0) {
						_id = e.column_value;
					}
					else if (e.column_name.compare("created_at") == 0) {
						_created_at = e.column_value;
					}
				}
			);

			ASSERT_STREQ2("3", _id);
			ASSERT_STREQ2(now.substr(0, -2).data(), _created_at);
		}

		// Should save food's description to specific locale.
		{
			std::string _title;
			std::string _description;

			stmt(
				fmt::format(
					"SELECT fl.title, fl.description FROM food_localisation fl "
					"WHERE fl.food_id = {} AND fl.locale_id = 1 ",
					_id
				),
				[&] (auto e) {
					if (e.column_name.compare("title") == 0) {
						_title = e.column_value;
					}
					else if (e.column_name.compare("description") == 0) {
						_description = e.column_value;
					}
				}
			);

			ASSERT_STREQ2("Tomato", _title);
			ASSERT_STREQ2("A red thing", _description);
		}

		// Should save nutrient info.
		{
			typedef std::tuple<std::string, std::string, std::string> _nut;
			std::vector<_nut> _nutrients {};
			_nut _nutrient {};

			stmt(
				fmt::format(
					"SELECT fn.food_id, fn.nutrient_id, fn.value "
					"FROM food_nutrient fn "
					"WHERE fn.food_id = {} ",
					_id
				),
				[&] (auto e) {
					if (e.column_name.compare("food_id") == 0) {
						std::get<0>(_nutrient) = e.column_value;
					}
					else if (e.column_name.compare("nutrient_id") == 0) {
						std::get<1>(_nutrient) = e.column_value;
					}
					else if (e.column_name.compare("value") == 0) {
						std::get<2>(_nutrient) = e.column_value;
						_nutrients.push_back(std::move(_nutrient));
						_nutrient = {};
					}
				}
			);

			ASSERT_EQ(3, _nutrients.size());

			std::array<std::array<std::string_view, 3>, 3> expected_nutrients {{
				{{"3", "1", "0.11"}},
				{{"3", "5", "42.4001"}},
				{{"3", "2", "33"}},
			}};
			size_t i = 0;
			for (const auto n : expected_nutrients) {
				ASSERT_STREQ2(n[0].data(), std::get<0>(_nutrients[i]));
				ASSERT_STREQ2(n[1].data(), std::get<1>(_nutrients[i]));
				ASSERT_STREQ2(n[2].data(), std::get<2>(_nutrients[i]));
				i++;
			}
		}

		// Should save info about directions.
		{
			std::string _recipe_step_id;
			std::string _recipe_step_time;
			std::string _recipe_step_description;

			stmt(
				fmt::format(
					"SELECT rs.id, rs.time, rsl.description "
					"FROM recipe_step rs "
					"INNER JOIN recipe_step_localisation rsl "
					" ON rsl.recipe_step_id = rs.id "
					" WHERE rsl.locale_id = 1 AND rs.recipe_id = {} ",
					_id
				),
				[&] (auto e) {
					if (e.column_name == "id") {
						_recipe_step_id = e.column_value;
					} else if (e.column_name == "time") {
						_recipe_step_time = e.column_value;
					} else if (e.column_name == "description") {
						_recipe_step_description = e.column_value;
					}
				}
			);

			ASSERT_STREQ2("1", _recipe_step_id);
			ASSERT_STREQ2("60", _recipe_step_time);
			ASSERT_STREQ2("do fry", _recipe_step_description);
		}
	}

	// When nutrients and directions are not provided.
	{
		wholth::entity::editable::Food food {
			.title = "Egg",
			.description = "A progeny in shell",
		};

		wholth::insert(db_con, food, "2");

		// Should create new row in `food`.
		std::string _id;
		{
			std::string _created_at;
			std::string now = wholth::utils::current_time_and_date();

			stmt(
				"SELECT f.id, f.created_at FROM food f",
				[&] (auto e) {
					if (e.column_name.compare("id") == 0) {
						_id = e.column_value;
					}
					else if (e.column_name.compare("created_at") == 0) {
						_created_at = e.column_value;
					}
				}
			);

			ASSERT_STREQ2("4", _id);
			ASSERT_STREQ2(now.substr(0, -2).data(), _created_at);
		}

		// Should save food's description to specific locale.
		{
			std::string _title;
			std::string _description;

			stmt(
				fmt::format(
					"SELECT fl.title, fl.description FROM food_localisation fl "
					"WHERE fl.food_id = {} AND fl.locale_id = 2 ",
					_id
				),
				[&] (auto e) {
					if (e.column_name.compare("title") == 0) {
						_title = e.column_value;
					}
					else if (e.column_name.compare("description") == 0) {
						_description = e.column_value;
					}
				}
			);

			ASSERT_STREQ2("Egg", _title);
			ASSERT_STREQ2("A progeny in shell", _description);
		}
	}

	// When food with same name exists in set locale.
	{

		{
			wholth::entity::editable::Food food {
				.title = "Bacon",
				.description = "fresh bacon"
			};

			wholth::insert(db_con, food, "2");
		}

		wholth::entity::editable::Food food {
			.title = "Bacon",
			.description = "unfresh bacon"
		};

		std::string prev_food_count;
		std::string last_food_id;
		stmt(
			"SELECT COUNT(f.id) cnt, MAX(f.id) mx FROM food f",
			[&](auto e) {
				if (e.column_name == "cnt") {
					prev_food_count = e.column_value;
				}
				else if (e.column_name == "mx") {
					last_food_id = e.column_value;
				}
			}
		);

		wholth::insert(db_con, food, "2");

		std::string cur_food_count;
		std::string cur_last_food_id;
		stmt(
			"SELECT COUNT(f.id) cnt, MAX(f.id) mx FROM food f",
			[&](auto e) {
				if (e.column_name == "cnt") {
					cur_food_count = e.column_value;
				}
				else if (e.column_name == "mx") {
					cur_last_food_id = e.column_value;
				}
			}
		);
		ASSERT_STREQ3(cur_food_count, prev_food_count);
		ASSERT_STREQ3(last_food_id, cur_last_food_id);

		// Should not save food's description to specific locale.
		{
			std::string _title;
			std::string _description;

			stmt(
				fmt::format(
					"SELECT fl.title, fl.description FROM food_localisation fl "
					"WHERE fl.food_id = {} AND fl.locale_id = 2 ",
					last_food_id
				),
				[&] (auto e) {
					if (e.column_name.compare("title") == 0) {
						_title = e.column_value;
					}
					else if (e.column_name.compare("description") == 0) {
						_description = e.column_value;
					}
				}
			);

			ASSERT_STREQ2("Bacon", _title);
			ASSERT_STRNEQ2("unfresh bacon", _description);
		}
	}

	// Should fail when nutrient value is not a float.
	{
		std::vector<wholth::entity::editable::food::Nutrient> nutrients {{
			{.id="5", .value="yooo0.11hola"},
		}};
		wholth::entity::editable::Food food {
			.title = "Acorn",
			.description = "a corn",
			.nutrients = nutrients,
		};

		std::string prev_food_count;
		std::string last_food_id;
		stmt(
			"SELECT COUNT(f.id) cnt, MAX(f.id) mx FROM food f",
			[&](auto e) {
				if (e.column_name == "cnt") {
					prev_food_count = e.column_value;
				}
				else if (e.column_name == "mx") {
					last_food_id = e.column_value;
				}
			}
		);

		wholth::insert(db_con, food, "2");

		std::string cur_food_count;
		std::string cur_last_food_id;
		stmt(
			"SELECT COUNT(f.id) cnt, MAX(f.id) mx FROM food f",
			[&](auto e) {
				if (e.column_name == "cnt") {
					cur_food_count = e.column_value;
				}
				else if (e.column_name == "mx") {
					cur_last_food_id = e.column_value;
				}
			}
		);
		ASSERT_STREQ3(cur_food_count, prev_food_count);
		ASSERT_STREQ3(last_food_id, cur_last_food_id);
	}
}
