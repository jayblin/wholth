#include <gtest/gtest.h>
#include "wholth/c/entity/food.h"
#include <unistd.h>

static_assert(nullptr == (void*)NULL);

GTEST_TEST(wholth_recipe_step_init, A)
{
    wholth_Food food = wholth_entity_food_init();
    ASSERT_EQ(food.id.data, nullptr);
    ASSERT_EQ(food.id.size, 0);
    ASSERT_EQ(food.title.data, nullptr);
    ASSERT_EQ(food.title.size, 0);
    ASSERT_EQ(food.preparation_time.data, nullptr);
    ASSERT_EQ(food.preparation_time.size, 0);
    ASSERT_EQ(food.top_nutrient.data, nullptr);
    ASSERT_EQ(food.top_nutrient.size, 0);
    ASSERT_EQ(food.description.data, nullptr);
    ASSERT_EQ(food.description.size, 0);
}
