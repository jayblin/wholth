#include "gtest/gtest.h"
#include <array>
#include <gtest/gtest.h>
#include <span>
#include <string>
#include <type_traits>
#include "helpers.hpp"
#include "sqlw/forward.hpp"
#include "sqlw/statement.hpp"
#include "wholth/buffer_view.hpp"
#include "wholth/context.hpp"
#include "wholth/list.hpp"
#include "wholth/model/foods_page.hpp"
#include "wholth/status.hpp"
#include "wholth/entity/food.hpp"

class ListFoodsTest : public MigrationAwareTest
{
protected:
	static void SetUpTestSuite()
	{
        MigrationAwareTest::SetUpTestSuite();
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
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
         ec = stmt(
            "INSERT INTO locale (id,alias) VALUES "
            "(1,'EN'),(2,'RU'),(3,'DE')"
        );
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
        ec = stmt("INSERT INTO food_localisation (food_id, locale_id, title, description) "
            "VALUES "
            "(1, 1, 'salt', 'Essence of salt'),"
            "(2, 2, 'Cacao', NULL),"
            "(5, 2, 'Salia', NULL),"
            "(5, 3, 'Saliagr', 'I dont''t know'),"
            "(1, 2, 'Salta', NULL),"
            "(4, 2, 'Saltabar', NULL)"
        );
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
        ec = stmt("INSERT INTO nutrient (id,unit,position) "
            "VALUES "
            "(1, 'a', 1),"
            "(2, 'b', 0),"
            "(3, 'c', 2)"
        );
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
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
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
        ec = stmt("INSERT INTO recipe_step (recipe_id,seconds) VALUES "
            "(1, '600') "
        );
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);

        std::string buffer1 {};
        std::string buffer2 {};
        std::string buffer3 {};
	}
};

TEST_F(ListFoodsTest, list_foods_1)
{
    wholth::Context ctx {};

    wholth::BufferView<std::array<wholth::entity::Food, 2>> bv {
        /* .view={}, */
        /* .buffer={} */
    };
    uint64_t count;
    /* wholth::list::food::Query q { */
    /* 	.page = 0, */
    /* 	.locale_id = "1", */
    /* }; */
    ctx.locale_id = "1";
    wholth::model::FoodsPage q {
        .ctx=ctx,
        /* .pagination{0} */
        .pagination={bv.view.size()},
    };
    auto ec = wholth::fill_span<wholth::entity::Food>(bv.view, bv.buffer, count, q, db_con);
    auto list = std::span{bv.view};

    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2("1", list[0].id);
    ASSERT_STREQ2("salt", list[0].title);
    ASSERT_STREQ2("10m", list[0].preparation_time);
    ASSERT_STREQ2("22.4 b", list[0].top_nutrient);
    ASSERT_STREQ2("2", list[1].id);
    ASSERT_STREQ2("[N/A]", list[1].title);
    ASSERT_STREQ2("[N/A]", list[1].preparation_time);
    ASSERT_STREQ2("19.0 b", list[1].top_nutrient);
    ASSERT_EQ(6, count);
}

TEST_F(ListFoodsTest, list_foods_2)
{
    wholth::Context ctx {};

    wholth::BufferView<std::array<wholth::entity::Food, 10>> bv {
        /* .view={}, */
        /* .buffer={} */
    };
    uint64_t count;
    /* wholth::list::food::Query q { */
    /* 	.page = 0, */
    /* 	.locale_id = "2", */
    /* 	.title = "Sal", */
    /* }; */
    ctx.locale_id = "2";
    wholth::model::FoodsPage q {
        .ctx=ctx,
        /* .pagination{0}, */
        .pagination={bv.view.size()},
        .title="Sal"
    };
    auto ec = wholth::fill_span<wholth::entity::Food>(bv.view, bv.buffer, count, q, db_con);
    auto list = std::span{bv.view};

    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec;
    ASSERT_STREQ2("1", list[0].id);
    ASSERT_STREQ2("Salta", list[0].title);
    ASSERT_STREQ2("10m", list[0].preparation_time);
    ASSERT_STREQ2("4", list[1].id);
    ASSERT_STREQ2("Saltabar", list[1].title);
    ASSERT_STREQ2("[N/A]", list[1].preparation_time);
    ASSERT_STREQ2("5", list[2].id);
    ASSERT_STREQ2("Salia", list[2].title);
    ASSERT_STREQ2("[N/A]", list[2].preparation_time);
    ASSERT_EQ(3, count);
}

TEST_F(ListFoodsTest, list_foods_3)
{
    wholth::Context ctx {};

    wholth::BufferView<std::array<wholth::entity::Food, 10>> bv {
        /* .view={}, */
        /* .buffer={} */
    };
    uint64_t count;
    /* wholth::list::food::Query q { */
    /* 	.page = 0, */
    /* 	.locale_id = "", */
    /* 	.title = "Sal", */
    /* }; */
    ctx.locale_id = "";
    wholth::model::FoodsPage q {
        .ctx=ctx,
        /* .pagination{0}, */
        .pagination={bv.view.size()},
        .title = "Sal",
    };
    auto ec = wholth::fill_span<wholth::entity::Food>(bv.view, bv.buffer, count, q, db_con);
    auto list = std::span{bv.view};


    ASSERT_FALSE(wholth::status::Condition::OK == ec) << ec;
    ASSERT_EQ(wholth::status::Code::INVALID_LOCALE_ID, ec) << ec;
    ASSERT_EQ(0, count);
}

TEST_F(ListFoodsTest, list_foods_4)
{
    wholth::Context ctx {};

    wholth::BufferView<std::array<wholth::entity::Food, 10>> bv {
        /* .view={}, */
        /* .buffer={} */
    };
    uint64_t count;
    /* wholth::list::food::Query q { */
    /* 	.page = 0, */
    /* 	.locale_id = "1a", */
    /* 	.title = "Sal", */
    /* }; */
    ctx.locale_id = "1a";
    wholth::model::FoodsPage q {
        .ctx=ctx,
        /* .pagination{0}, */
        .pagination={bv.view.size()},
        .title = "Sal",
    };
    auto ec = wholth::fill_span<wholth::entity::Food>(bv.view, bv.buffer, count, q, db_con);
    auto list = std::span{bv.view};

    ASSERT_EQ(wholth::status::Code::INVALID_LOCALE_ID, ec) << ec;
    ASSERT_EQ(0, count);
}

TEST_F(ListFoodsTest, list_foods_5)
{
    wholth::Context ctx {};

    wholth::BufferView<std::array<wholth::entity::Food, 2>> bv {
        /* .view={}, */
        /* .buffer={} */
    };
    uint64_t count;
    /* wholth::list::food::Query q { */
    /* 	.page = 1, */
    /* 	.locale_id = "2", */
    /* 	.title = "Sal", */
    /* }; */
    ctx.locale_id = "2";
    wholth::model::FoodsPage q {
        .ctx=ctx,
        /* .pagination{0}, */
        .pagination={bv.view.size()},
        .title = "Sal",
    };
    q.pagination.advance();
    auto ec = wholth::fill_span<wholth::entity::Food>(bv.view, bv.buffer, count, q, db_con);
    auto list = std::span{bv.view};

    ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec;
    // Checks that foods are sorted by id by default.
    ASSERT_STREQ2("5", list[0].id);
    ASSERT_STREQ2("Salia", list[0].title);
    ASSERT_STREQ2("[N/A]", list[0].preparation_time);
    ASSERT_EQ(3, count);
}

TEST_F(ListFoodsTest, list_foods_6)
{
    /* std::array<wholth::list::food::nutrient_filter::Entry, 500> arr {{ */
    /* 	{wholth::list::food::nutrient_filter::Operation::EQ, "1", "100"}, */
    /* 	{wholth::list::food::nutrient_filter::Operation::NEQ, "2", "22.4"}, */
    /* 	{wholth::list::food::nutrient_filter::Operation::BETWEEN, "3", "10.3,40"}, */
    /* }}; */
    /* std::array<wholth::entity::Food, 3> list; */
    /* wholth::list::food::Query q { */
    /* 	.page = 0, */
    /* 	.locale_id = "1", */
    /* 	.nutrient_filters = arr, */
    /* }; */
    /* uint64_t count; */
    /* wholth::StatusCode rc1 = wholth::fill_span(list, count, buffer1, q, db_con); */

    /* ASSERT_EQ(wholth::StatusCode::NO_ERROR, rc1) << wholth::view(rc1); */
    /* ASSERT_STREQ2("3", list[0].id); */
    /* ASSERT_STREQ2("4", list[1].id); */
    /* ASSERT_STREQ2("", list[2].id); */
    /* ASSERT_EQ(2, count); */
}

	// switch buffer check
TEST_F(ListFoodsTest, list_foods_7)
{
    wholth::Context ctx {};

    /* std::array<wholth::entity::Food, 4> list; */
    /* std::string buffer1_1 {}; */
    /* std::string buffer1_2 {}; */
    /* wholth::list::food::Query q { */
    /* 	.page = 0, */
    /* 	.locale_id = "1", */
    /* }; */

    wholth::BufferView<std::array<wholth::entity::Food, 4>> bv {
        /* .view={}, */
        /* .buffer={} */
    };
    uint64_t count;
    auto list = std::span{bv.view.begin(), 2};
    ctx.locale_id = "1";
    wholth::model::FoodsPage q {
        .ctx=ctx,
        /* .pagination{0} */
        .pagination={list.size()},
    };
    auto ec1 = wholth::fill_span<wholth::entity::Food>(list, bv.buffer, count, q, db_con);

    ASSERT_TRUE(wholth::status::Condition::OK == ec1) << ec1;
    ASSERT_STREQ2("1", list[0].id);
    ASSERT_STREQ2("salt", list[0].title);
    ASSERT_STREQ2("10m", list[0].preparation_time);
    ASSERT_STREQ2("2", list[1].id);
    ASSERT_STREQ2("[N/A]", list[1].title);
    ASSERT_STREQ2("[N/A]", list[1].preparation_time);
    ASSERT_EQ(6, count);


    std::string buffer;
    /* q.page = 1; */
    q.pagination.advance();
    list = std::span{bv.view.begin() + 2, 2};
    /* const auto ia = bv.view.size(); */
    auto ec2 = wholth::fill_span(list, buffer, count, q, db_con);

    ASSERT_TRUE(wholth::status::Condition::OK == ec2) << ec2;
    ASSERT_STREQ2("3", bv.view[2].id);
    ASSERT_STREQ2("[N/A]", bv.view[2].title);
    ASSERT_STREQ2("[N/A]", bv.view[2].preparation_time);
    ASSERT_STREQ2("4", bv.view[3].id);
    ASSERT_STREQ2("[N/A]", bv.view[3].title);
    ASSERT_STREQ2("[N/A]", bv.view[3].preparation_time);
    ASSERT_EQ(6, count);
}

class ListFoodsTest2 : public MigrationAwareTest
{
protected:
	static void SetUpTestSuite()
	{
        MigrationAwareTest::SetUpTestSuite();
        sqlw::Statement stmt {&db_con};

        auto ec = stmt("INSERT INTO food (id, created_at) "
            "VALUES "
            " (1,'10-10-2010'),"
            " (2,'10-10-2010'),"
            " (3,'10-10-2010'),"
            " (4,'10-10-2010'),"
            " (5,'10-10-2010'),"
            " (6,'10-10-2010'),"
            " (7,'10-10-2010')"
        );
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
        ec = stmt(
            "INSERT INTO locale (id,alias) VALUES "
            "(1,'EN'),(2,'RU'),(3,'DE')"
        );
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
        ec = stmt("INSERT INTO food_localisation (food_id, locale_id, title) "
            "VALUES "
            "(1, 3, 'scrambled eggs'), "
            "(2, 3, 'eggs'), "
            "(3, 3, 'salt'), "
            "(4, 3, 'butter'), "
            "(5, 3, 'sugar'), "
            "(6, 3, 'water'), "
            "(7, 3, 'soda') "
        );
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
        // todo case when recipe contains it's ingredient name.
        ec = stmt("INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
            "(100, 1, '3720'), "
            "(200, 1, '60'), "
            "(300, 7, '340') "
        );
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
        ec = stmt("INSERT INTO recipe_step_food (recipe_step_id,food_id,canonical_mass) VALUES "
            "(100, 2, 21), "
            "(200, 3, 31), "
            "(200, 4, 41), "
            "(300, 5, 52), "
            "(300, 6, 62) "
        );
        ASSERT_TRUE(sqlw::status::Condition::OK == ec);
	}
};

TEST_F(ListFoodsTest2, list_foods_by_ingredients)
{
    wholth::Context ctx {};

	{
		std::string buffer1;
		std::array<wholth::entity::Food, 10> list;
		/* wholth::list::food::Query q { */
		/* 	.page = 0, */
		/* 	.locale_id = "3", */
		/* 	.ingredients = "water,salt", */
		/* }; */
        ctx.locale_id = "3";
        wholth::model::FoodsPage q {
            .ctx=ctx,
            /* .pagination{0}, */
            .pagination={list.size()},
			.ingredients = "water,salt",
        };
        uint64_t count;
        wholth::BufferView<std::span<wholth::entity::Food>> bv {.view=list};
        /* const auto ia = bv.view.size(); */
		auto ec = wholth::fill_span<wholth::entity::Food>(std::span{list}, buffer1, count, q, db_con);

		ASSERT_TRUE(wholth::status::Condition::OK == ec) << ec;
	}
}
