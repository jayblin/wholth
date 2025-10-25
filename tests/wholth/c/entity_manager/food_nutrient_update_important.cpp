#include "db/db.hpp"
#include "helpers.hpp"
#include "sqlw/statement.hpp"
#include "wholth/c/buffer.h"
#include "wholth/c/entity/food.h"
#include "wholth/c/entity_manager/food_nutrient.h"
#include "wholth/c/forward.h"
#include "wholth/entity_manager/food.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <type_traits>

static_assert(nullptr == (void*)NULL);

using bind_t = sqlw::Statement::bindable_t;

class Test_wholth_em_food_nutrient_update_important
    : public ApplicationAwareTest
{
  protected:
    void SetUp()
    {
        ApplicationAwareTest::SetUp();
        auto& con = db::connection();

        // 1
        // -1- 2
        // -1- 3
        // -1- 4
        // -1-3- 7
        // -1-3-4- 8
        // -2- 4
        // -2- 5
        // -2- 6
        // 10
        // -5- 11
        astmt(
            con,
            "INSERT INTO recipe_step (id,recipe_id,seconds) VALUES "
            "(1,1,600),"
            "(2,1,600),"
            "(3,4,600),"
            "(4,7,600),"
            "(5,10,600)");

        astmt(
            con,
            "INSERT INTO recipe_step_food (recipe_step_id, food_id, "
            "canonical_mass) VALUES "
            "(1,2,200),"
            "(1,3,300),"
            "(1,4,400),"
            "(2,4,400),"
            "(2,5,500),"
            "(2,6,600),"
            "(3,7,700),"
            "(4,8,800),"
            "(5,11,1100)");

        astmt(
            con,
            "INSERT INTO nutrient (id,unit,position) VALUES "
            "(1,'KCAL',10),"
            "(2,'KCAL',11),"
            "(3,'G',12),"
            "(4,'G',20),"
            "(5,'G',30),"
            "(6,'G',40),"
            "(7,'G',41),"
            "(8,'G',50),"
            "(9,'G',60),"
            "(10,'G',61) "
            "ON CONFLICT DO UPDATE SET "
            "unit=excluded.unit, position=excluded.unit");
        astmt(con, "DELETE FROM food_nutrient");
        astmt(
            con,
            "INSERT INTO food_nutrient(food_id,nutrient_id,value) VALUES "
            "(2,1, 20),"
            "(2,2, 20),"
            "(2,3, 20),"
            "(2,4, 20),"
            "(2,5, 20),"
            "(2,6, 20),"
            "(2,7, 20),"
            "(2,8, 20),"
            "(2,9, 20),"
            "(2,10,20),"

            "(3,1, 30),"
            "(3,2, 30),"
            /* "(3,3, 30)," */
            "(3,4, 30),"
            "(3,5, 30),"
            "(3,6, 30),"
            "(3,7, 30),"
            "(3,8, 30),"
            "(3,9, 30),"
            "(3,10,30),"

            "(4,1, 40),"
            "(4,2, 40),"
            "(4,3, 40),"
            "(4,4, 40),"
            /* "(4,5, 40)," */
            "(4,6, 40),"
            "(4,7, 40),"
            "(4,8, 40),"
            "(4,9, 40),"
            "(4,10,40),"

            "(5,1, 50),"
            "(5,2, 50),"
            "(5,3, 50),"
            "(5,4, 50),"
            "(5,5, 50),"
            "(5,6, 50),"
            "(5,7, 50),"
            "(5,8, 50),"
            "(5,9, 50),"
            "(5,10,50),"

            "(6,1, 60),"
            "(6,2, 60),"
            "(6,3, 60),"
            "(6,4, 60),"
            "(6,5, 60),"
            "(6,6, 60),"
            "(6,7, 60),"
            "(6,8, 60),"
            "(6,9, 60),"
            "(6,10,60),"

            "(7,1, 70),"
            "(7,2, 70),"
            "(7,3, 70),"
            "(7,4, 70),"
            "(7,5, 70),"
            "(7,6, 70),"
            "(7,7, 70),"
            "(7,8, 70),"
            "(7,9, 70),"
            "(7,10,70),"

            "(8,1, 80),"
            "(8,2, 80),"
            "(8,3, 80),"
            "(8,4, 80),"
            "(8,5, 80),"
            "(8,6, 80),"
            "(8,7, 80),"
            "(8,8, 80),"
            "(8,9, 80),"
            "(8,10,80),"

            "(9,1, 90),"
            "(9,2, 90),"
            "(9,3, 90),"
            "(9,4, 90),"
            "(9,5, 90),"
            "(9,6, 90),"
            "(9,7, 90),"
            "(9,8, 90),"
            "(9,9, 90),"
            "(9,10,90),"

            "(10,1, 100),"
            "(10,2, 100),"
            "(10,3, 100),"
            "(10,4, 100),"
            "(10,5, 100),"
            "(10,6, 100),"
            "(10,7, 100),"
            "(10,8, 100),"
            "(10,9, 100),"
            "(10,10,100),"

            "(11,3, 1103)");
    }
};

TEST_F(Test_wholth_em_food_nutrient_update_important, when_basic_case)
{
    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("1");
    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_food_nutrient_update_important(&food, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::stringstream ss;
    astmt(
        db::connection(),
        R"sql(
        SELECT
            n.id AS id,
            ROUND(fn.value, 2) AS value
        FROM food_nutrient fn
        INNER JOIN nutrient n
            ON n.id = fn.nutrient_id
        WHERE fn.food_id = 1
        ORDER BY n.position ASC
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    // Amount of nutrient in 100 grams of recipe is:
    // Sigma(CanonicalMass_i * NutrientValue_i)
    //       ----------------------------
    //          Sigma(CanonicalMass_i)
    // Where:
    // - CanonicalMass_i is mass of ingredient number i in a recipe step;
    // - NutrientValue_i is the amount of nutrient in 100 grams of
    //   ingredient number i in a recipe step.
    //
    //   Example for nutrient id = 1:
    // (200*20 + 300*30 + 400*40 + 400*40 + 500*50 + 600*60)
    //      -----------------------------------------
    //         (200 + 300 + 400 + 400 + 500 + 600)
    //
    //
    // The function must assume that each ingredient has its nutrient value
    // already calculated, so recursion only 1 level deep.
    ASSERT_STREQ2(
        "id:1;value:44.17;"
        "id:2;value:44.17;"
        "id:4;value:44.17;"
        "id:6;value:44.17;"
        "id:7;value:44.17;"
        "id:8;value:44.17;"
        "id:9;value:44.17;",
        ss.str());
}

TEST_F(Test_wholth_em_food_nutrient_update_important, when_basic_case_2)
{
    {
        std::stringstream ss;
        astmt(
            db::connection(),
            R"sql(
        SELECT
            n.id AS id,
            ROUND(fn.value, 2) AS value
        FROM food_nutrient fn
        INNER JOIN nutrient n
            ON n.id = fn.nutrient_id
        WHERE fn.food_id = 10
        ORDER BY n.position ASC
        )sql",
            [&](auto e) {
                ss << e.column_name << ":" << e.column_value << ';';
            });
        ASSERT_STREQ2(
            "id:1;value:100;"
            "id:2;value:100;"
            "id:3;value:100;"
            "id:4;value:100;"
            "id:5;value:100;"
            "id:6;value:100;"
            "id:7;value:100;"
            "id:8;value:100;"
            "id:9;value:100;"
            "id:10;value:100;",
            ss.str());
    }

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("10");
    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_food_nutrient_update_important(&food, buf);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::stringstream ss;
    astmt(
        db::connection(),
        R"sql(
        SELECT
            n.id AS id,
            ROUND(fn.value, 2) AS value
        FROM food_nutrient fn
        INNER JOIN nutrient n
            ON n.id = fn.nutrient_id
        WHERE fn.food_id = 10
        ORDER BY n.position ASC
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_STREQ2(
        "id:1;value:100;"
        "id:2;value:100;"
        "id:3;value:1103;"
        "id:4;value:100;"
        "id:5;value:100;"
        "id:6;value:100;"
        "id:7;value:100;"
        "id:8;value:100;"
        "id:9;value:100;"
        "id:10;value:100;",
        ss.str());
}

TEST_F(Test_wholth_em_food_nutrient_update_important, when_food_is_nulltpr)
{
    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_food_nutrient_update_important(NULL, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::FOOD_NULL, ec);
}

TEST_F(Test_wholth_em_food_nutrient_update_important, when_food_id_is_nulltpr)
{
    wholth_Food food = wholth_entity_food_init();
    wholth_Buffer* buf = wholth_buffer_ring_pool_element();
    const auto err = wholth_em_food_nutrient_update_important(&food, buf);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::error_code ec = wholth::entity_manager::food::Code(err.code);
    ASSERT_EQ(wholth::entity_manager::food::Code::FOOD_INVALID_ID, ec);
}

TEST_F(Test_wholth_em_food_nutrient_update_important, when_buffer_is_nulltpr)
{
    {
        std::stringstream ss;
        astmt(
            db::connection(),
            R"sql(
        SELECT
            n.id AS id,
            ROUND(fn.value, 2) AS value
        FROM food_nutrient fn
        INNER JOIN nutrient n
            ON n.id = fn.nutrient_id
        WHERE fn.food_id = 10
        ORDER BY n.position ASC
        )sql",
            [&](auto e) {
                ss << e.column_name << ":" << e.column_value << ';';
            });
        ASSERT_STREQ2(
            "id:1;value:100;"
            "id:2;value:100;"
            "id:3;value:100;"
            "id:4;value:100;"
            "id:5;value:100;"
            "id:6;value:100;"
            "id:7;value:100;"
            "id:8;value:100;"
            "id:9;value:100;"
            "id:10;value:100;",
            ss.str());
    }

    wholth_Food food = wholth_entity_food_init();
    food.id = wtsv("10");
    const auto err = wholth_em_food_nutrient_update_important(&food, NULL);

    ASSERT_NE(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    std::stringstream ss;
    astmt(
        db::connection(),
        R"sql(
        SELECT
            n.id AS id,
            ROUND(fn.value, 2) AS value
        FROM food_nutrient fn
        INNER JOIN nutrient n
            ON n.id = fn.nutrient_id
        WHERE fn.food_id = 10
        ORDER BY n.position ASC
        )sql",
        [&](auto e) { ss << e.column_name << ":" << e.column_value << ';'; });
    ASSERT_STREQ2(
        "id:1;value:100;"
        "id:2;value:100;"
        "id:3;value:100;"
        "id:4;value:100;"
        "id:5;value:100;"
        "id:6;value:100;"
        "id:7;value:100;"
        "id:8;value:100;"
        "id:9;value:100;"
        "id:10;value:100;",
        ss.str());
}
