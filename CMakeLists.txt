cmake_minimum_required(VERSION 3.23)

project(wholth VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(
	GCC_COMPILER_FLAGS
	"-g;-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused"
)
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")

set(VULKAN_SDK_INCLUDE_DIR "0" CACHE FILEPATH "Path to the vulkan sdk inlude directory")

if (VULKAN_SDK_INCLUDE_DIR STREQUAL "0")
    message(FATAL_ERROR "You must provide a value for VULKAN_SDK_INCLUDE_DIR variable!")
endif()

set(VULKAN_SDK_LIBRARY_PATH "0" CACHE FILEPATH "Path to the vulkan sdk library object")

if (VULKAN_SDK_LIBRARY_PATH STREQUAL "0")
    message(FATAL_ERROR "You must provide a value for VULKAN_SDK_LIBRARY_PATH variable!")
endif()

set(DEFAULT_FONT_PATH "" CACHE FILEPATH "Path to the default ttf font")

add_compile_definitions(SQLITE_ENABLE_FTS5)

list(
    APPEND
    WHOLTH_SOURCE_FILES
    src/utils/json_serializer.cpp
	src/utils.cpp
	src/vk/vk.cpp
    src/vk/instance.cpp
    src/vk/physical_device.cpp
    src/vk/device.cpp
    src/vk/swapchain.cpp
    src/vk/descriptor_pool.cpp
    src/vk/render_pass.cpp
    src/ui/glfw_window.cpp
    src/ui/imgui.cpp
	src/db/db.cpp
    src/wholth/page.cpp
    src/wholth/context.cpp
    src/wholth/status.cpp
	src/entity/food.cpp
	src/list/food.cpp
    src/ui/glfw_platform.cpp
    src/ui/glfw_window.cpp
    src/ui/imgui.cpp
    src/ui/platform.cpp
    src/ui/components/food_list.cpp
    src/ui/components/food_edit_modal.cpp
    src/ui/components/foods_page.cpp
    src/wholth/controller/expanded_food.cpp
    src/wholth/controller/foods_page.cpp
    src/wholth/app.cpp
)

# add_executable(wholth ${WHOLTH_SOURCE_FILES} src/main_4.cpp)

add_library(
	wholth
	SHARED
    ${WHOLTH_SOURCE_FILES}
)

target_compile_options(
	wholth
	PUBLIC "$<${gcc_like_cxx}:${GCC_COMPILER_FLAGS}>"
)

include(FetchContent)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.12.1
)

FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.3.8
)

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

FetchContent_Declare(
  imgui_src
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.89
)

# @todo: uncomment later
FetchContent_MakeAvailable(glfw sqlw imgui_src fmt)

FetchContent_GetProperties(
	imgui_src
	SOURCE_DIR SRC_DIR_IMGUI
	BINARY_DIR BIN_DIR_IMGUI
	POPULATED POPULATED_IMGUI
)

FetchContent_GetProperties(
	glfw
	SOURCE_DIR SRC_DIR_GLFW
	BINARY_DIR BIN_DIR_GLFW
	POPULATED POPULATED_GLFW
)

FetchContent_GetProperties(
	gsl
	SOURCE_DIR SRC_DIR_GSL
	BINARY_DIR BIN_DIR_GSL
	POPULATED POPULATED_GSL
)

add_library(
	imgui
	SHARED
	${SRC_DIR_IMGUI}/imgui.cpp
	${SRC_DIR_IMGUI}/imgui_tables.cpp
	${SRC_DIR_IMGUI}/imgui_widgets.cpp
	${SRC_DIR_IMGUI}/imgui_draw.cpp
	${SRC_DIR_IMGUI}/imgui_demo.cpp # remove when in prod
	${SRC_DIR_IMGUI}/backends/imgui_impl_vulkan.cpp
	${SRC_DIR_IMGUI}/backends/imgui_impl_glfw.cpp
)

target_include_directories(
	imgui
	PUBLIC
        ${VULKAN_SDK_INCLUDE_DIR}
		${SRC_DIR_IMGUI}
		${SRC_DIR_GLFW}/include
		${SRC_DIR_GSL}/include
)

target_link_libraries(
	imgui
	glfw
	Microsoft.GSL::GSL
    ${VULKAN_SDK_LIBRARY_PATH}
)

# target_link_libraries(
# 	Microsoft.GSL::GSL
# )

target_include_directories(
	wholth
	PUBLIC
		include
        ${VULKAN_SDK_INCLUDE_DIR}
		${SRC_DIR_IMGUI}
		${SRC_DIR_GLFW}/include
)

target_link_libraries(
	wholth
    ${VULKAN_SDK_LIBRARY_PATH}
	sqlw
	Microsoft.GSL::GSL
	imgui
    fmt
)

# TESTS

# https://google.github.io/googletest/quickstart-cmake.html

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_custom_target(wholth_tests_t)

add_executable(
	wholth_tests
	src/db/db.cpp
    src/wholth/status.cpp
	src/entity/food.cpp
	src/list/food.cpp
	src/utils.cpp
	tests/food.cpp
    tests/db/db.cpp
    tests/helpers.cpp
)

target_link_libraries(
	wholth_tests
	PRIVATE
		GTest::gtest_main
		sqlw
		fmt
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
