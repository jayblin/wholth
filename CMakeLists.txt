cmake_minimum_required(VERSION 3.23)

project(Wholth VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(SQLITE_ENABLE_FTS5)

add_library(
	wholth
	SHARED
    # STATIC
    # src/utils/json_serializer.cpp
	src/utils.cpp
	src/db/db.cpp
    src/wholth/page.cpp
    src/wholth/context.cpp
    src/wholth/status.cpp
	src/entity/food.cpp
	src/list/food.cpp
	src/list/nutrient.cpp
    src/wholth/app.cpp
    src/wholth/scheduler.cpp
    src/wholth/app_c.cpp
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
  GIT_TAG        93f91846ead377aca6c2dfa46044bf895a160695
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


target_include_directories(
	wholth
	PUBLIC
		include
)

target_link_libraries(
	wholth
	sqlw
	Microsoft.GSL::GSL
    fmt
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
        tests/helpers.cpp
        tests/db/db.cpp
        tests/list/nutrient.cpp
        tests/list/food.cpp
        tests/food.cpp
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

# add_custom_target(wholth_install_t)

# add_executable(
# 	wholth_install
# 	src/db/db.cpp
# 	src/install.cpp
# )

# target_link_libraries(
# 	wholth_install
# 	PRIVATE
# 		sqlw
# )

# add_dependencies(wholth_install_t wholth_install)

# target_include_directories(
# 	wholth_install
# 	PUBLIC
# 		include
# )

# install(
# 	TARGETS
# 		sqlw
# 		# wholth_install
# 		wholth_tests
# 		wholth
# 	RUNTIME
# 	DESTINATION bin
# 	OPTIONAL
# )

# ~INSTALL

configure_file(
	${PROJECT_SOURCE_DIR}/include/wholth/cmake_vars.h.in
	${PROJECT_SOURCE_DIR}/include/wholth/cmake_vars.h
)
