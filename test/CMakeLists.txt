include(${CMAKE_SOURCE_DIR}/target_link_libraries_system.cmake)

add_executable(test ${test_sources}
detail/csr.cpp
detail/neuron_population.cpp
detail/synapse_population.cpp
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

target_compile_options(test PRIVATE ${spice_warning_flags} ${spice_math_flags})
target_link_libraries_system(test PRIVATE spice gtest_main hana)