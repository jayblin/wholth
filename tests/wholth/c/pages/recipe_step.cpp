#include "gtest/gtest.h"
#include <type_traits>
#include "helpers.hpp"
#include "wholth/c/app.h"
#include "wholth/c/entity/recipe_step.h"
#include "wholth/c/pages/utils.h"
#include "wholth/c/pages/recipe_step.h"
#include "wholth/pages/code.hpp"

static_assert(nullptr == (void*)NULL);

class Test_wholth_pages_recipe_step : public ApplicationAwareTest
{
  protected:
    static void SetUpTestSuite()
    {
        ApplicationAwareTest::SetUpTestSuite();
        astmt(db::connection(), "SAVEPOINT Test_wholth_pages_recipe_step");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO recipe_step "
            " (id, recipe_id, priority, seconds) VALUES "
            " (99991, 22, 1, 600),"
            " (99992, 22, 0, 500)");
        astmt(
            db::connection(),
            "INSERT OR REPLACE INTO recipe_step_localisation "
            " (recipe_step_id, locale_id, description) VALUES "
            " (99992, 1, 'do one and then do two!')");
    }

    static void TearDownTestSuite()
    {
        astmt(db::connection(), "ROLLBACK TO Test_wholth_pages_recipe_step");
    }
};

TEST_F(Test_wholth_pages_recipe_step, when_basic_case)
{
    wholth_user_locale_id(wtsv("1"));

    wholth_Page* page = wholth_pages_recipe_step(true);

    wholth_pages_recipe_step_recipe_id(page, wtsv("22"));

    // ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    const wholth_Error err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const wholth_RecipeStep* step = wholth_pages_recipe_step_first(page);

    ASSERT_NE(nullptr, step);

    ASSERT_STREQ2("99992", wfsv(step->id));
    ASSERT_STREQ2("8m", wfsv(step->time));
    ASSERT_STREQ2("do one and then do two!", wfsv(step->description));
}

TEST_F(Test_wholth_pages_recipe_step, when_basic_case_and_diff_locale)
{
    wholth_user_locale_id(wtsv("2"));

    wholth_Page* page = wholth_pages_recipe_step(true);

    wholth_pages_recipe_step_recipe_id(page, wtsv("22"));

    // ASSERT_TRUE(wholth_pages_skip_to(page, 0));
    const wholth_Error err = wholth_pages_fetch(page);

    ASSERT_EQ(wholth_Error_OK.code, err.code) << err.code << wfsv(err.message);
    ASSERT_EQ(wholth_Error_OK.message.size, err.message.size)
        << err.code << wfsv(err.message);

    const wholth_RecipeStep* step = wholth_pages_recipe_step_first(page);

    ASSERT_NE(nullptr, step);

    ASSERT_STREQ2("99992", wfsv(step->id));
    ASSERT_STREQ2("8m", wfsv(step->time));
    ASSERT_STREQ2("[N/A]", wfsv(step->description));
}

TEST_F(Test_wholth_pages_recipe_step, when_not_found)
{
    {
        wholth_Page* page = wholth_pages_recipe_step(true);

        wholth_user_locale_id(wtsv("2"));
        wholth_pages_recipe_step_recipe_id(page, wtsv("22"));
        wholth_user_locale_id(wtsv("1"));
        wholth_pages_recipe_step_recipe_id(page, wtsv("22"));
    }

    {
        wholth_user_locale_id(wtsv("1"));

        wholth_Page* page = wholth_pages_recipe_step(false);

        wholth_pages_recipe_step_recipe_id(page, wtsv("1"));

        // ASSERT_TRUE(wholth_pages_skip_to(page, 0));
        const wholth_Error err = wholth_pages_fetch(page);

        ASSERT_NE(wholth_Error_OK.code, err.code)
            << err.code << wfsv(err.message);
        ASSERT_NE(wholth_Error_OK.message.size, err.message.size)
            << err.code << wfsv(err.message);

        const std::error_code ec = wholth::pages::Code(err.code);
        ASSERT_EQ(wholth::pages::Code::NOT_FOUND, ec) << ec << ec.message();

        const wholth_RecipeStep* step = wholth_pages_recipe_step_first(page);

        ASSERT_EQ(nullptr, step);
    }
}
