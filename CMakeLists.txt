cmake_minimum_required(VERSION 3.12)

project(spice VERSION 0.2.0 DESCRIPTION "Spice SNN Simulator")
set(CMAKE_CXX_STANDARD 20)

set(spice_assert_preconditions TRUE CACHE BOOL "Assert preconditions in release mode (recommended)")
set(spice_assert_invariants TRUE CACHE BOOL "Assert invariants in release mode (for developers only)")
set(spice_use_matplot FALSE CACHE BOOL "Use matplotlib for visualization inside the samples. Alternatively, print json to the command line. (optional)")
set(spice_build_tests FALSE CACHE BOOL "Build unit tests? (optional, for developers only)")
set(spice_build_benchmarks FALSE CACHE BOOL "Build performance benchmarks? (optional, for developers only)")

# Global flags
if (CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_definitions(NDEBUG)
endif()

# Common compile options shared by multiple projects
set(spice_warning_flags
-pedantic
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
-Wno-sign-compare
-Wno-implicit-fallthrough
-Wno-overloaded-virtual
-Werror
-Wno-error=conversion
-Wno-error=float-conversion
-Wno-error=sign-conversion
-fconcepts-diagnostics-depth=2
)

set(spice_math_flags
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

# Third-party dependencies
add_subdirectory(third_party)

# spice (snn simulation library)
add_subdirectory(spice)

# samples
add_subdirectory(samples)

# test (unit test suite)
if(spice_build_tests)
	add_subdirectory(test)
endif()

# bench (performance benchmark suite)
if(spice_build_benchmarks)
	add_subdirectory(bench)
endif()
