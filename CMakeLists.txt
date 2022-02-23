cmake_minimum_required(VERSION 3.1)

project(spice VERSION 0.1 DESCRIPTION "Spice SNN Simulator")

# Make gcc compile with pthreads support
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Common compile options shared by multiple projects
set(warning_flags
-Werror
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
-Wconversion
-Wsign-conversion
-Wnull-dereference
-Wuseless-cast)

set(math_flags
-ffast-math #-fno-math-errno -funsafe-math-optimizations -ffinite-math-only -fno-rounding-math -fno-signaling-nans -fcx-limited-range -fexcess-precision=fast
-fassociative-math
-freciprocal-math
-fno-signed-zeros
-fno-trapping-math
-march=native
-mfpmath=sse)

# spice (snn simulation library)
add_library(spice SHARED spice/spice.h)
set_target_properties(spice PROPERTIES LINKER_LANGUAGE CXX CXX_STANDARD 20)
target_compile_options(spice PRIVATE ${warning_flags} ${math_flags})

# test (unit test suite)
set(spice_build_tests FALSE CACHE BOOL "Build unit tests? (optional, for developers only)")
if(spice_build_tests)
	add_executable(test test/main.cpp)
	target_include_directories(test PRIVATE . third_party/googletest/googletest/include)
	target_link_libraries(test PRIVATE spice Threads::Threads ${CMAKE_SOURCE_DIR}/third_party/googletest/build/lib/libgtest.a ${CMAKE_SOURCE_DIR}/third_party/googletest/build/lib/libgtest_main.a)
endif()

# bench (performance benchmark suite)
set(spice_build_benchmarks FALSE CACHE BOOL "Build performance benchmarks? (optional, for developers only)")
if(spice_build_benchmarks)
	add_executable(bench bench/main.cpp)
	target_include_directories(bench PRIVATE . third_party/benchmark/include)
	target_link_libraries(bench PRIVATE spice Threads::Threads ${CMAKE_SOURCE_DIR}/third_party/benchmark/build/src/libbenchmark.a ${CMAKE_SOURCE_DIR}/third_party/benchmark/build/src/libbenchmark_main.a)
endif()

# lamodel (sample application using spice)
add_executable(lamodel lamodel/main.cpp)
set_target_properties(lamodel PROPERTIES LINKER_LANGUAGE CXX CXX_STANDARD 20)
target_compile_options(lamodel PRIVATE ${warning_flags} ${math_flags})
target_include_directories(lamodel PRIVATE .)
target_link_libraries(lamodel PRIVATE spice Threads::Threads)