#include "utils/convert.hpp"
#include <gtest/gtest.h>
#include <limits>

typedef std::tuple<
    std::string_view, // case_name
    std::string_view, // value
    uint,             // value to compare result to
    bool              // is expected to return error?
    >
    test_case_t;

class Test_utils_convert_to_uint : public testing::TestWithParam<test_case_t>
{
  protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_P(Test_utils_convert_to_uint, cases)
{
    const auto [_, value, expected_result, is_expecting_error] = GetParam();

    uint result;
    const std::error_code ec = utils::convert::to_uint(value, result);
    ASSERT_EQ(expected_result, result);

    if (is_expecting_error)
    {
        ASSERT_TRUE(ec);
    }
    else
    {
        ASSERT_TRUE(!ec);
    }
}

INSTANTIATE_TEST_SUITE_P(
    Instantiantion,
    Test_utils_convert_to_uint,
    testing::Values(
        test_case_t{"A", "2025", 2025, false},
        test_case_t{"B", "2025-", 0, true},
        test_case_t{"C", "", 0, true},
        test_case_t{"D", {}, 0, true},
        test_case_t{"E", "1", 1, false},
        test_case_t{"F", "12", 12, false},
        test_case_t{"G", "123", 123, false},
        test_case_t{"H", "1234", 1234, false},
        test_case_t{"I", "12345", 12345, false},
        test_case_t{"J", "123456", 123456, false},
        test_case_t{"K", "-12", 0, true},
        test_case_t{"L", "12.3", 0, true},
        test_case_t{"M", "0.4", 0, true},
        test_case_t{"N", "0,45", 0, true},
        test_case_t{"O", "abfgqo32", 0, true},
        test_case_t{"P", "0", 0, false},
        test_case_t{"Q", "4294967295", 4294967295, false},
        test_case_t{"S", "4294967296", 0, true},
        test_case_t{"T", "4294967296222222", 0, true}),
    [](const testing::TestParamInfo<Test_utils_convert_to_uint::ParamType>& i) {
        return std::string{std::get<0>(i.param)};
    });
