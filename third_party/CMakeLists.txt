if(spice_use_matplot)
	set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build matplot samples" FORCE)
	set(BUILD_HIGH_RESOLUTION_WORLD_MAP OFF CACHE BOOL "Disable matplot hires world map" FORCE)
	set(BUILD_INSTALLER OFF CACHE BOOL "Do not install 3rd party dependencies" FORCE)
	set(BUILD_PACKAGE OFF CACHE BOOL "Do not package matplot" FORCE)
	set(BUILD_TESTING OFF CACHE BOOL "Do not build matplot tests" FORCE)
	set(BUILD_TESTS OFF CACHE BOOL "Do not build matplot tests" FORCE)
	set(BUILD_WITH_MSVC_HACKS OFF CACHE BOOL "Disable matplot msvc hacks" FORCE)
	add_subdirectory(matplot)
endif()

if(spice_build_tests)
	set(INSTALL_GTEST OFF CACHE BOOL "Do not install 3rd party dependencies" FORCE)
	set(BUILD_GMOCK OFF CACHE BOOL "Do not build gmock" FORCE)
	set(gtest_disable_pthreads ON CACHE BOOL "Disable pthreads in googletest" FORCE)
	add_subdirectory(googletest)
endif()

if(spice_build_benchmarks)
	set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Do not install 3rd party dependencies" FORCE)
	set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "Do not install gbench docs" FORCE)
	set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Do not build gbench unit tests" FORCE)
	set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Do not build gbench unit tests" FORCE)
	set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "Disable -werror inside gbench" FORCE)
	add_subdirectory(benchmark)
endif()

add_subdirectory(hana)