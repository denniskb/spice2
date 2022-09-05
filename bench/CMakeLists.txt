add_executable(bench
connectivity.cpp)

target_compile_options(bench PRIVATE ${spice_warning_flags} ${spice_math_flags})
target_link_libraries(bench PRIVATE spice benchmark_main)