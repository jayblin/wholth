cmake_minimum_required(VERSION 3.23)

project(Wholth VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(SQLITE_ENABLE_FTS5)

add_library(
	wholth
	SHARED
    # STATIC
	# src/list/food.cpp
	# src/list/food_details.cpp
	# src/list/food_ingredients.cpp
	# src/list/ingredients.cpp
	# src/list/nutrient.cpp
	src/db/db.cpp
	# src/entity/food.cpp
	src/utils.cpp
    # src/controller/insert_ingredient.cpp
    # src/controller/insert_recipe_step.cpp
    # src/controller/update_food.cpp
    # src/utils/json_serializer.cpp
    # src/wholth/c/food_details.cpp
    # src/wholth/c/food_indredients.cpp
    # src/wholth/c/food_nutreints.cpp
    # src/wholth/c/foods_page.cpp
    # src/wholth/c/ingredients.cpp
    # src/wholth/c/recipe_step.cpp
    # src/wholth/scheduler.cpp
    # todo move these to other target
    src/utils/datetime.cpp
    src/utils/is_valid_id.cpp
    src/utils/time_to_seconds.cpp
    src/wholth/app.cpp
    src/wholth/c/app.cpp
    # src/wholth/pages/internal.cpp
    src/wholth/c/buffer.cpp
    src/wholth/error.cpp
    src/wholth/c/entity/food.cpp
    src/wholth/c/entity/recipe_step.cpp
    src/wholth/c/entity/ingredient.cpp
    src/wholth/c/entity/nutrient.cpp
    src/wholth/c/entity/consumption_log.cpp
    src/wholth/c/entity/user.cpp
    src/wholth/c/entity_manager/ingredient.cpp
    src/wholth/c/entity_manager/recipe_step.cpp
    src/wholth/c/entity_manager/food.cpp
    src/wholth/c/entity_manager/ingredient.cpp
    src/wholth/c/entity_manager/food_nutrient.cpp
    src/wholth/c/entity_manager/consumption_log.cpp
    src/wholth/c/entity_manager/user.cpp
    src/wholth/c/pages/food.cpp
    src/wholth/c/pages/food_nutrient.cpp
    src/wholth/c/pages/nutrient.cpp
    src/wholth/c/pages/ingredient.cpp
    src/wholth/c/pages/consumption_log.cpp
    src/wholth/c/pages/recipe_step.cpp
    src/wholth/c/pages/utils.cpp
    src/wholth/c/internal.cpp
    src/wholth/c/string_view.cpp
    src/wholth/pages/internal.cpp
    src/wholth/pages/food.cpp
    # src/wholth/context.cpp
    src/wholth/entity_manager/ingredient.cpp
    src/wholth/entity_manager/nutrient.cpp
    src/wholth/entity_manager/recipe_step.cpp
    # src/wholth/entity_manager/update_important_nutrients.cpp
    src/wholth/pagination.cpp
    src/wholth/status.cpp
    src/wholth/utils/length_container.cpp
    src/wholth/utils/prepend_sql_params.cpp
    src/wholth/utils/to_error.cpp
    src/wholth/utils/str_replace.cpp
    src/wholth/utils/str_split.cpp
    src/wholth/c/exec_stmt.cpp
)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(WHOLTH_STANDALONE "1")
endif()

# todo Also check if standalone then dont use these???
# if (CMAKE_CXX_COMPILER STREQUAL "Clang" OR CMAKE_C_COMPILER STREQUAL "Clang")
#     target_compile_options(
#         wholth
#         PUBLIC "-Wall;-Wextra;-Wshadow-all"
#     )
# elseif (CMAKE_CXX_COMPILER STREQUAL "GNU" OR CMAKE_C_COMPILER STREQUAL "GNU")
#     target_compile_options(
#         wholth
#         PUBLIC "-Wall;-Wextra;-Wshadow;-Wformat=2"
#     )
# endif()

include(FetchContent)

if (DEFINED WHOLTH_STANDALONE)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
endif()

FetchContent_Declare(
  sqlw
  GIT_REPOSITORY https://github.com/jayblin/sqlw.git
  GIT_TAG        86da5145f3e34eb0c8ae7dc947c515c3232bef3d
)

FetchContent_Declare(
  gsl
  GIT_REPOSITORY https://github.com/microsoft/GSL.git
  GIT_TAG        v4.0.0
)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        10.1.1
)

FetchContent_MakeAvailable(sqlw fmt)

FetchContent_GetProperties(
	gsl
	SOURCE_DIR SRC_DIR_GSL
	BINARY_DIR BIN_DIR_GSL
	POPULATED POPULATED_GSL
)

find_package(OpenSSL 3.5 COMPONENTS Crypto)

target_include_directories(
	wholth
	PUBLIC
		include
)

target_link_libraries(
    wholth
    PRIVATE sqlw Microsoft.GSL::GSL fmt OpenSSL::Crypto
)

# TESTS
if (DEFINED WHOLTH_STANDALONE)
    # https://google.github.io/googletest/quickstart-cmake.html

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_custom_target(wholth_tests_t)

    add_executable(
        wholth_tests
        # tests/controller/ingredients.cpp
        # tests/controller/insert_food.cpp
        # tests/controller/insert_ingredient.cpp
        # tests/controller/insert_recipe_step.cpp
        # tests/controller/update_food.cpp
        # tests/controller/update_important_nutrients.cpp
        # tests/db/db.cpp !!!!!!!!uncomment!!!!!!!!
        # tests/food.cpp
        # tests/list/food.cpp
        # tests/list/nutrient.cpp
        tests/utils/time_to_seconds.cpp
        tests/utils/convert.cpp
        tests/utils/datetime.cpp
        tests/helpers.cpp
        tests/wholth/c/entity/food.cpp
        tests/wholth/c/entity_manager/food_insert.cpp
        tests/wholth/c/entity_manager/food_update.cpp
        tests/wholth/c/entity_manager/ingredient_insert.cpp
        tests/wholth/c/entity_manager/ingredient_update.cpp
        tests/wholth/c/entity_manager/food_nutrient_upsert.cpp
        tests/wholth/c/entity_manager/food_nutrient_update_important.cpp
        tests/wholth/c/entity_manager/consumption_log_delete.cpp
        tests/wholth/c/entity_manager/consumption_log_insert.cpp
        tests/wholth/c/entity_manager/consumption_log_update.cpp
        tests/wholth/c/entity_manager/recipe_step_insert.cpp
        tests/wholth/c/entity_manager/recipe_step_update.cpp
        tests/wholth/c/entity_manager/user_insert.cpp
        tests/wholth/c/entity_manager/user_authenticate.cpp
        tests/wholth/c/pages/food.cpp
        tests/wholth/c/pages/food_nutrient.cpp
        tests/wholth/c/pages/nutrient.cpp
        tests/wholth/c/pages/ingredient.cpp
        tests/wholth/c/pages/consumption_log.cpp
        tests/wholth/c/pages/recipe_step.cpp
        tests/wholth/sql_statements/ingredient_delete.cpp
        # tests/wholth/c/entity_manager/recipe_step_update.cpp
        # tests/wholth/c/entity_manager/update_food.cpp
        # tests/wholth/c/entity_manager/update_important_nutrients.cpp
        # tests/wholth/c/entity_manager/update_ingredient.cpp
        # tests/wholth/c/entity_manager/upsert_food_nutrient.cpp
    )

    target_link_libraries(
        wholth_tests
        PRIVATE
            GTest::gtest_main
            sqlw
            fmt
            wholth
    )

    target_include_directories(
        wholth_tests
        PUBLIC
            include
            tests
    )

    add_dependencies(wholth_tests_t wholth_tests)

    include(GoogleTest)

    # gtest_discover_tests(wholth_tests)
endif()
#~TESTS

# INSTALL

install(
	TARGETS wholth sqlw
    LIBRARY COMPONENT Runtime
)

# ~INSTALL

configure_file(
	${PROJECT_SOURCE_DIR}/include/wholth/cmake_vars.h.in
	${PROJECT_SOURCE_DIR}/include/wholth/cmake_vars.h
)
