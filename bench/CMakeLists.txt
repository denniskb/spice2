add_executable(bench
brunel.cpp
brunel+.cpp
connectivity.cpp)

target_compile_options(bench PRIVATE ${warning_flags} ${math_flags})
target_include_directories(bench PRIVATE ${CMAKE_SOURCE_DIR}/spice/include ${CMAKE_SOURCE_DIR}/third_party/benchmark/include)
target_link_libraries(bench PRIVATE spice benchmark_main)