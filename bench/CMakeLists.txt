add_executable(bench
connectivity.cpp)

target_compile_options(bench PRIVATE ${spice_warning_flags} ${spice_math_flags})
target_include_directories(bench SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/third_party/benchmark/include)
target_include_directories(bench PRIVATE ${CMAKE_SOURCE_DIR}/spice/include)
target_link_libraries(bench PRIVATE spice benchmark_main)