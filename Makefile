all:
	@echo "Hello there!"

# DEV
prep-dev-clang:
	cmake -B build_dev -S . --toolchain toolchain_clang.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug

prep-dev-gnu:
	cmake -B build_dev -S . --toolchain toolchain_gnu.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug

build-dev:
	cmake --build build_dev --target=wholth_tests
	cp build_dev/compile_commands.json ./.

install-dev:
	cmake --install build_dev --verbose --component Runtime --strip

test-dev:
	./build_dev/wholth_tests --gtest_color=no --gtest_filter=$(TEST_CASE)
# ~DEV

# RELEASE
prep-release:
	cmake -B build_release -S . --toolchain toolchain_gnu.cmake -DCMAKE_BUILD_TYPE=Release

build-release:
	cmake --build build_release --target=wholth

install-release:
	cmake --install build_release --verbose --component Runtime --strip
# ~RELEASE
