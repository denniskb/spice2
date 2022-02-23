cmake_minimum_required(VERSION 3.1)

project(spice VERSION 0.1 DESCRIPTION "Spice SNN Simulator")

add_library(spice SHARED spice/spice.h)
set_target_properties(spice PROPERTIES LINKER_LANGUAGE CXX CXX_STANDARD 20)
target_compile_options(spice PRIVATE
-O2
-march=native
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

set(spice_build_tests FALSE CACHE BOOL "Build unit tests? (optional, for developers only)")
if(spice_build_tests)
	add_executable(test test/main.cpp)
	target_include_directories(test PRIVATE . third_party/googletest/googletest/include)
	target_link_libraries(test PRIVATE spice ${CMAKE_SOURCE_DIR}/third_party/googletest/build/lib/libgtest.a ${CMAKE_SOURCE_DIR}/third_party/googletest/build/lib/libgtest_main.a)
endif()

add_executable(lamodel lamodel/main.cpp)
target_include_directories(lamodel PRIVATE .)
target_link_libraries(lamodel PRIVATE spice)