add_executable(test ${test_sources}
detail/csr.cpp
detail/neuron_population.cpp
#detail/synapse_population.cpp
util/assert.cpp
util/meta.cpp
util/numeric.cpp
util/random.cpp
util/range.cpp
util/scope.cpp
util/stdint.cpp
util/type_traits.cpp
concepts.cpp
snn.cpp)

target_compile_options(test PRIVATE ${warning_flags} ${math_flags})
target_include_directories(test PRIVATE ${CMAKE_SOURCE_DIR}/spice/include ${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include)
target_link_libraries(test PRIVATE spice gtest_main)