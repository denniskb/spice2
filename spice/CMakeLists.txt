# Enable multi-threading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(spice SHARED
detail/csr.h
detail/neuron_population.h
detail/synapse_population.h
util/assert.cpp
util/assert.h
util/meta.h
util/numeric.h
util/random.h
util/range.h
util/scope.h
util/stdint.h
util/type_traits.h
concepts.h
connectivity.cpp
connectivity.h
sim_info.h
snn.cpp
snn.h)

target_compile_options(spice PRIVATE ${warning_flags} ${math_flags})
target_include_directories(spice PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(spice PUBLIC Threads::Threads)

if(spice_assert_preconditions)
	target_compile_definitions(spice PUBLIC SPICE_ASSERT_PRECONDITIONS)
endif()
if(spice_assert_invariants)
	target_compile_definitions(spice PUBLIC SPICE_ASSERT_INVARIANTS)
endif()