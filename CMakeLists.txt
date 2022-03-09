cmake_minimum_required(VERSION 3.12)

project(spice VERSION 0.1 DESCRIPTION "Spice SNN Simulator")
set(CMAKE_CXX_STANDARD 20)

# Make gcc compile with pthreads support
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Common compile options shared by multiple projects
set(warning_flags
-Wall
-Wextra
-Wshadow
-Wnon-virtual-dtor
-pedantic
-Wpedantic
-Wold-style-cast
-Wcast-align
-Wunused
-Woverloaded-virtual
-Wnull-dereference
-Wuseless-cast
-Werror
-Wno-error=conversion
-Wno-error=float-conversion
-Wno-error=sign-conversion
-Wno-error=sign-compare
-Wno-sign-compare
-fconcepts-diagnostics-depth=2
# Optional, additional (noisy) warnings (won't affect compilation):
#-Wsign-compare
#-Wconversion
#-Wsign-conversion
)

set(math_flags
-ffast-math # Enables: -fno-math-errno -funsafe-math-optimizations -ffinite-math-only -fno-rounding-math -fno-signaling-nans -fcx-limited-range -fexcess-precision=fast
-fassociative-math
-freciprocal-math
-fno-signed-zeros
-fno-trapping-math
-march=haswell
-mfpmath=sse
# TODO: Try these
#-mfma
#-mavx(2)
#-funroll-loops
)

# spice (snn simulation library)
file(GLOB_RECURSE spice_sources spice/*.h spice/*.cpp)
add_library(spice SHARED ${spice_sources})
target_compile_options(spice PRIVATE ${warning_flags} ${math_flags})

# lamodel (sample application using spice)
file(GLOB_RECURSE lamodel_sources lamodel/*.h lamodel/*.cpp)
add_executable(lamodel ${lamodel_sources})
target_compile_options(lamodel PRIVATE ${warning_flags} ${math_flags})
target_include_directories(lamodel PRIVATE . third_party/matplot/source)
target_link_libraries(lamodel PRIVATE spice Threads::Threads ${CMAKE_SOURCE_DIR}/third_party/matplot/build/source/matplot/libmatplot.a ${CMAKE_SOURCE_DIR}/third_party/matplot/build/source/3rd_party/libnodesoup.a)

# test (unit test suite)
set(spice_build_tests FALSE CACHE BOOL "Build unit tests? (optional, for developers only)")
if(spice_build_tests)
	file(GLOB_RECURSE test_sources test/*.h test/*.cpp)
	add_executable(test ${test_sources})
	target_compile_options(test PRIVATE ${warning_flags} ${math_flags})
	target_include_directories(test PRIVATE . third_party/googletest/googletest/include)
	target_link_libraries(test PRIVATE spice Threads::Threads ${CMAKE_SOURCE_DIR}/third_party/googletest/build/lib/libgtest.a ${CMAKE_SOURCE_DIR}/third_party/googletest/build/lib/libgtest_main.a)
endif()

# bench (performance benchmark suite)
set(spice_build_benchmarks FALSE CACHE BOOL "Build performance benchmarks? (optional, for developers only)")
if(spice_build_benchmarks)
	file(GLOB_RECURSE bench_sources bench/*.h bench/*.cpp)
	add_executable(bench ${bench_sources})
	target_compile_options(bench PRIVATE ${warning_flags} ${math_flags})
	target_include_directories(bench PRIVATE . third_party/benchmark/include)
	target_link_libraries(bench PRIVATE spice Threads::Threads ${CMAKE_SOURCE_DIR}/third_party/benchmark/build/src/libbenchmark.a ${CMAKE_SOURCE_DIR}/third_party/benchmark/build/src/libbenchmark_main.a)
endif()