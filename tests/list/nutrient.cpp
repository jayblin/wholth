#include "wholth/list/nutrient.hpp"
#include "gtest/gtest.h"
#include <array>
#include <gtest/gtest.h>
#include <limits>
#include <span>
#include <type_traits>
#include "helpers.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/list.hpp"
#include "wholth/status.hpp"
#include "wholth/entity/food.hpp"

TEST_F(MigrationAwareTest, fill_span_with_nutrients)
{
	sqlw::Statement stmt {&db_con};

	auto ec = stmt("INSERT INTO food (id, created_at) "
		"VALUES "
		" (1,'10-10-2010'),"
		" (2,'10-10-2010'),"
		" (3,'10-10-2010'),"
		" (4,'10-10-2010'),"
		" (5,'10-10-2010'),"
		" (6,'10-10-2010')"
	);
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
	 ec = stmt(
		"INSERT INTO locale (id,alias) VALUES "
		"(1,'EN'),(2,'RU'),(3,'DE')"
	);
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
	ec = stmt("INSERT INTO nutrient (id,unit,position) "
		"VALUES "
		"(1, 'a', 1),"
		"(2, 'b', 0),"
		"(3, 'c', 2)"
	);
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
	ec = stmt("INSERT INTO food_nutrient (food_id, nutrient_id, value) "
		"VALUES "
		"(1, 1, 100),"
		"(1, 2, 22.4),"
		"(1, 3, 20),"
		"(2, 1, 99),"
		"(2, 2, 19),"
		"(2, 3, 20),"
		"(3, 1, 100),"
		"(3, 2, 20),"
		"(3, 3, 10.33),"
		"(4, 1, 100),"
		"(4, 2, 30),"
		"(4, 3, 39.999),"
		"(5, 1, 10),"
		"(5, 2, 20),"
		"(5, 3, 20),"
		"(6, 1, 100),"
		"(6, 2, 0),"
		"(6, 3, 50)"
	);
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;
	ec = stmt("INSERT INTO nutrient_localisation (nutrient_id,locale_id,title) "
		"VALUES "
		"(1, 1, 'A'),"
		"(2, 1, 'B'),"
		"(3, 1, 'C')"
	);
    ASSERT_TRUE(sqlw::status::Condition::OK == ec) << ec;

    // No locale in query
	{
        wholth::BufferView<std::array<wholth::entity::Nutrient, 2>> bv {
            /* .view={}, */
            /* .buffer={} */
        };
        uint64_t count;
		wholth::list::nutrient::FoodDependentQuery q {
			.page = 0,
		};
		ec = wholth::fill_span<wholth::entity::Nutrient>(bv.view, bv.buffer, count, q, db_con);
        auto list = std::span{bv.view};

		ASSERT_TRUE(wholth::status::Code::INVALID_LOCALE_ID == ec) << ec;
		ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
		ASSERT_STREQ2("", list[0].id);
		ASSERT_STREQ2("", list[0].title);
		ASSERT_STREQ2("", list[0].value);
		ASSERT_STREQ2("", list[0].unit);
		ASSERT_STREQ2("", list[0].position);
		ASSERT_STREQ2("", list[1].id);
		ASSERT_STREQ2("", list[1].title);
		ASSERT_STREQ2("", list[1].value);
		ASSERT_STREQ2("", list[1].unit);
		ASSERT_STREQ2("", list[1].position);
        ASSERT_EQ(0, count);
	}

    // No food id in query
	{
        wholth::BufferView<std::array<wholth::entity::Nutrient, 2>> bv {
            /* .view={}, */
            /* .buffer={} */
        };
        uint64_t count;
		wholth::list::nutrient::FoodDependentQuery q {
			.page = 0,
			.locale_id = "1",
		};
		ec = wholth::fill_span<wholth::entity::Nutrient>(bv.view, bv.buffer, count, q, db_con);
        auto list = std::span{bv.view};

		ASSERT_TRUE(wholth::status::Code::INVALID_FOOD_ID == ec) << ec;
		ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
		ASSERT_STREQ2("", list[0].id);
		ASSERT_STREQ2("", list[0].title);
		ASSERT_STREQ2("", list[0].value);
		ASSERT_STREQ2("", list[0].unit);
		ASSERT_STREQ2("", list[0].position);
		ASSERT_STREQ2("", list[1].id);
		ASSERT_STREQ2("", list[1].title);
		ASSERT_STREQ2("", list[1].value);
		ASSERT_STREQ2("", list[1].unit);
		ASSERT_STREQ2("", list[1].position);
        ASSERT_EQ(0, count);
	}

    // Invalid locale in query
	{
        wholth::BufferView<std::array<wholth::entity::Nutrient, 2>> bv {
            /* .view={}, */
            /* .buffer={} */
        };
        uint64_t count;
		wholth::list::nutrient::FoodDependentQuery q {
			.page = 0,
            .locale_id = "12f",
		};
		ec = wholth::fill_span<wholth::entity::Nutrient>(bv.view, bv.buffer, count, q, db_con);
        auto list = std::span{bv.view};

		ASSERT_TRUE(wholth::status::Code::INVALID_LOCALE_ID == ec) << ec;
		ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
		ASSERT_STREQ2("", list[0].id);
		ASSERT_STREQ2("", list[0].title);
		ASSERT_STREQ2("", list[0].value);
		ASSERT_STREQ2("", list[0].unit);
		ASSERT_STREQ2("", list[0].position);
		ASSERT_STREQ2("", list[1].id);
		ASSERT_STREQ2("", list[1].title);
		ASSERT_STREQ2("", list[1].value);
		ASSERT_STREQ2("", list[1].unit);
		ASSERT_STREQ2("", list[1].position);
        ASSERT_EQ(0, count);
	}

    // Inlvalid id in query
	{
        wholth::BufferView<std::array<wholth::entity::Nutrient, 2>> bv {
            /* .view={}, */
            /* .buffer={} */
        };
        uint64_t count;
		wholth::list::nutrient::FoodDependentQuery q {
			.page = 0,
			.locale_id = "1",
            .food_id = "388e",
		};
		ec = wholth::fill_span<wholth::entity::Nutrient>(bv.view, bv.buffer, count, q, db_con);
        auto list = std::span{bv.view};

		ASSERT_TRUE(wholth::status::Code::INVALID_FOOD_ID == ec) << ec;
		ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
		ASSERT_STREQ2("", list[0].id);
		ASSERT_STREQ2("", list[0].title);
		ASSERT_STREQ2("", list[0].value);
		ASSERT_STREQ2("", list[0].unit);
		ASSERT_STREQ2("", list[0].position);
		ASSERT_STREQ2("", list[1].id);
		ASSERT_STREQ2("", list[1].title);
		ASSERT_STREQ2("", list[1].value);
		ASSERT_STREQ2("", list[1].unit);
		ASSERT_STREQ2("", list[1].position);
        ASSERT_EQ(0, count);
	}

    // Requested page number is too big for int.
	{
        wholth::BufferView<std::array<wholth::entity::Nutrient, 2>> bv {
            /* .view={}, */
            /* .buffer={} */
        };
        uint64_t count;
		wholth::list::nutrient::FoodDependentQuery q {
			.page = std::numeric_limits<uint64_t>::max(),
			.locale_id = "1",
            .food_id = "388e",
		};
		ec = wholth::fill_span<wholth::entity::Nutrient>(bv.view, bv.buffer, count, q, db_con);
        auto list = std::span{bv.view};

		ASSERT_TRUE(wholth::status::Code::QUERY_PAGE_TOO_BIG == ec) << ec;
		ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
		ASSERT_STREQ2("", list[0].id);
		ASSERT_STREQ2("", list[0].title);
		ASSERT_STREQ2("", list[0].value);
		ASSERT_STREQ2("", list[0].unit);
		ASSERT_STREQ2("", list[0].position);
		ASSERT_STREQ2("", list[1].id);
		ASSERT_STREQ2("", list[1].title);
		ASSERT_STREQ2("", list[1].value);
		ASSERT_STREQ2("", list[1].unit);
		ASSERT_STREQ2("", list[1].position);
        ASSERT_EQ(0, count);
	}

    // Requested offset (page*span.size) is too big for int.
	{
        wholth::BufferView<std::array<wholth::entity::Nutrient, 3>> bv {
            /* .view={}, */
            /* .buffer={} */
        };
        uint64_t count;
		wholth::list::nutrient::FoodDependentQuery q {
			.page = std::numeric_limits<int>::max()/2,
			.locale_id = "1",
            .food_id = "388",
		};
		ec = wholth::fill_span<wholth::entity::Nutrient>(bv.view, bv.buffer, count, q, db_con);
        auto list = std::span{bv.view};

		ASSERT_TRUE(wholth::status::Code::QUERY_OFFSET_TOO_BIG == ec) << ec;
		ASSERT_TRUE(wholth::status::Condition::OK != ec) << ec;
		ASSERT_STREQ2("", list[0].id);
		ASSERT_STREQ2("", list[0].title);
		ASSERT_STREQ2("", list[0].value);
		ASSERT_STREQ2("", list[0].unit);
		ASSERT_STREQ2("", list[0].position);
		ASSERT_STREQ2("", list[1].id);
		ASSERT_STREQ2("", list[1].title);
		ASSERT_STREQ2("", list[1].value);
		ASSERT_STREQ2("", list[1].unit);
		ASSERT_STREQ2("", list[1].position);
        ASSERT_EQ(0, count);
	}

    // Basic use case.
	{
        wholth::BufferView<std::array<wholth::entity::Nutrient, 2>> bv {
            /* .view={}, */
            /* .buffer={} */
        };
        uint64_t count;
		wholth::list::nutrient::FoodDependentQuery q {
			.page = 0,
			.locale_id = "1",
            .food_id = "3",
		};
		ec = wholth::fill_span<wholth::entity::Nutrient>(bv.view, bv.buffer, count, q, db_con);
        auto list = std::span{bv.view};

		/* ASSERT_TRUE(wholth::status::Code::INVALID_FOOD_ID == ec) << ec; */
		ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec;
		ASSERT_STREQ2("2", list[0].id);
		ASSERT_STREQ2("B", list[0].title);
		ASSERT_STREQ2("20", list[0].value);
		ASSERT_STREQ2("b", list[0].unit);
		ASSERT_STREQ2("0", list[0].position);
		ASSERT_STREQ2("1", list[1].id);
		ASSERT_STREQ2("A", list[1].title);
		ASSERT_STREQ2("100", list[1].value);
		ASSERT_STREQ2("a", list[1].unit);
		ASSERT_STREQ2("1", list[1].position);
        ASSERT_EQ(3, count) << "Total count of nutreints for specified food";
	}

    // Basic use case. Span size is larger than total element count.
	{
        wholth::BufferView<std::array<wholth::entity::Nutrient, 4>> bv {
            /* .view={}, */
            /* .buffer={} */
        };
        uint64_t count;
		wholth::list::nutrient::FoodDependentQuery q {
			.page = 0,
			.locale_id = "1",
            .food_id = "3",
		};
		ec = wholth::fill_span<wholth::entity::Nutrient>(bv.view, bv.buffer, count, q, db_con);
        auto list = std::span{bv.view};

		ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec;
		ASSERT_STREQ2("2", list[0].id);
		ASSERT_STREQ2("B", list[0].title);
		ASSERT_STREQ2("20", list[0].value);
		ASSERT_STREQ2("b", list[0].unit);
		ASSERT_STREQ2("0", list[0].position);
		ASSERT_STREQ2("1", list[1].id);
		ASSERT_STREQ2("A", list[1].title);
		ASSERT_STREQ2("100", list[1].value);
		ASSERT_STREQ2("a", list[1].unit);
		ASSERT_STREQ2("1", list[1].position);
		ASSERT_STREQ2("3", list[2].id);
		ASSERT_STREQ2("C", list[2].title);
		ASSERT_STREQ2("10.3", list[2].value);
		ASSERT_STREQ2("c", list[2].unit);
		ASSERT_STREQ2("2", list[2].position);
		ASSERT_STREQ2("", list[3].id);
		ASSERT_STREQ2("", list[3].title);
		ASSERT_STREQ2("", list[3].value);
		ASSERT_STREQ2("", list[3].unit);
		ASSERT_STREQ2("", list[3].position);
        ASSERT_EQ(3, count) << "Total count of nutreints for specified food";
	}
}
