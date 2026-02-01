# prep-release-clang:
# 	cmake -B build_release -S . --toolchain toolchain_clang.cmake -DCMAKE_BUILD_TYPE=Release
#
# prep-release-gnu:
# 	cmake -B build_release -S . --toolchain toolchain_gnu.cmake -DCMAKE_BUILD_TYPE=Release

prep-release:
	cmake -B build_release -S . -DCMAKE_BUILD_TYPE=Release

build-release:
	cmake --build build_release --target=wholth

install-release:
	cmake --install build_release --verbose --component Runtime --strip
