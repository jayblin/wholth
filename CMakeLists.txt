cmake_minimum_required(VERSION 3.23)

project(wholth VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(
	GCC_COMPILER_FLAGS
	"-g;-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused"
)
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")

add_executable(
	wholth
	# src/main.cpp
	src/main_2.cpp
)

target_include_directories(
	wholth
	PUBLIC
		include
		C:/VulkanSDK/1.3.250.0/Include
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
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 0.9.9.8
)

FetchContent_Declare(
  sqlw
  GIT_REPOSITORY https://github.com/jayblin/sqlw.git
  GIT_TAG        383d140bae7cc96b351fe370c9b8fdb4aee34aba
)

FetchContent_MakeAvailable(glfw glm sqlw)

target_link_libraries(
	wholth
	glfw
	glm
	sqlw
	C:/VulkanSDK/1.3.250.0/Lib/vulkan-1.lib
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
	tests/main.cpp
	# tests/main_2.cpp
)

target_link_libraries(
	wholth_tests
	PRIVATE
		GTest::gtest_main
		sqlw
)

target_include_directories(
	wholth
	PUBLIC
		include
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

install(
	TARGETS
		sqlw
		# wholth_install
		wholth_tests
		wholth
	RUNTIME
	DESTINATION bin
	OPTIONAL
)

# ~INSTALL

configure_file(
	${PROJECT_SOURCE_DIR}/include/wholth/cmake_vars.h.in
	${PROJECT_SOURCE_DIR}/include/wholth/cmake_vars.h
)
