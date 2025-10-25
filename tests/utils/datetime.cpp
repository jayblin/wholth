#include "utils/datetime.hpp"
#include <gtest/gtest.h>

typedef std::tuple<
    std::string_view, // case_name
    std::string_view, // value
    bool              // condition to compare result to
    >
    test_case_t;

class Test_utils_datetime_is_valid_sqlite_datetime
    : public testing::TestWithParam<test_case_t>
{
  protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_P(Test_utils_datetime_is_valid_sqlite_datetime, equivalence)
{
    const auto [_, value, expected_result] = GetParam();

    bool result = utils::datetime::is_valid_sqlite_datetime(value);
    ASSERT_EQ(expected_result, result);
}

INSTANTIATE_TEST_SUITE_P(
    Instantiantion,
    Test_utils_datetime_is_valid_sqlite_datetime,
    testing::Values(
        test_case_t{"A", "2025-08-04T23:14:55", true},
        test_case_t{"B", "2025-32-04T23:14:55", false},
        test_case_t{"C", "", false},
        test_case_t{"D", {}, false},
        test_case_t{"E", "2025-32-04", false},
        test_case_t{"F", "23:14:55", false},
        test_case_t{"G", "0000-01-01T00:00:00", true},
        test_case_t{"H", "2025-12-04T23-14-55", false},
        test_case_t{"I", "2025.12.04T23:14:55", false},
        test_case_t{"J", "bogus", false}),
    [](const testing::TestParamInfo<
        Test_utils_datetime_is_valid_sqlite_datetime::ParamType>& i) {
        return std::string{std::get<0>(i.param)};
    });
