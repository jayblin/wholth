#include "utils/time_to_seconds.hpp"
#include "helpers.hpp"
#include <gtest/gtest.h>

GTEST_TEST(time_to_seconds, A)
{
    std::string result;
    auto ec = utils::time_to_seconds("7h04m23s", result);

    ASSERT_TRUE(utils::time_to_seconds_Code::OK == ec) << ec << ec.message();
    /* (7 * 60 * 60) + (4 * 60) + 23; */
    ASSERT_STREQ2("25463", result);
}

GTEST_TEST(time_to_seconds, B)
{
    std::string result;
    auto ec = utils::time_to_seconds("7h 04m 23s", result);

    ASSERT_TRUE(utils::time_to_seconds_Code::OK == ec) << ec << ec.message();
    /* (7 * 60 * 60) + (4 * 60) + 23; */
    ASSERT_STREQ2("25463", result);
}

GTEST_TEST(time_to_seconds, C)
{
    std::string result;
    auto ec = utils::time_to_seconds("7h 3 04m 23s", result);

    ASSERT_TRUE(utils::time_to_seconds_Code::UNITLESS_VALUE == ec)
        << ec << ec.message();
    ASSERT_STREQ2("", result);
}

GTEST_TEST(time_to_seconds, D)
{
    std::string result;
    auto ec = utils::time_to_seconds("7h   04m    23s", result);

    ASSERT_TRUE(utils::time_to_seconds_Code::OK == ec) << ec << ec.message();
    /* (7 * 60 * 60) + (4 * 60) + 23; */
    ASSERT_STREQ2("25463", result);
}

GTEST_TEST(time_to_seconds, E)
{
    std::string result;
    auto ec = utils::time_to_seconds("  04m7h23s  ", result);

    ASSERT_TRUE(utils::time_to_seconds_Code::OK == ec) << ec << ec.message();
    /* (7 * 60 * 60) + (4 * 60) + 23; */
    ASSERT_STREQ2("25463", result);
}

GTEST_TEST(time_to_seconds, F)
{
    std::string result;
    auto ec = utils::time_to_seconds("1d04m7h23s", result);

    ASSERT_TRUE(utils::time_to_seconds_Code::INVALID_TIME_FORMAT == ec)
        << ec << ec.message();
    ASSERT_STREQ2("", result);
}

GTEST_TEST(time_to_seconds, G)
{
    std::string result;
    auto ec = utils::time_to_seconds("7000000000h04m23s", result);

    ASSERT_TRUE(utils::time_to_seconds_Code::VALUE_IS_TOO_LARGE == ec)
        << ec << ec.message();
    ASSERT_STREQ2("", result);
}

GTEST_TEST(time_to_seconds, H)
{
    std::string result;
    auto ec =
        utils::time_to_seconds("700000000h4000000000m2399999900s", result);

    ASSERT_TRUE(utils::time_to_seconds_Code::VALUE_IS_TOO_LARGE == ec)
        << ec << ec.message();
    ASSERT_STREQ2("", result);
}
