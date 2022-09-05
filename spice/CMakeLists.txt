# Enable multi-threading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(spice SHARED
include/spice/detail/csr.h
include/spice/detail/neuron_population.h
include/spice/detail/synapse_population.h
include/spice/util/assert.h
include/spice/util/meta.h
include/spice/util/numeric.h
include/spice/util/random.h
include/spice/util/range.h
include/spice/util/scope.h
include/spice/util/stdint.h
include/spice/util/type_traits.h
include/spice/concepts.h
include/spice/topology.h
include/spice/snn.h

src/util/assert.cpp
src/topology.cpp
src/snn.cpp)

target_compile_options(spice PRIVATE ${spice_warning_flags} ${spice_math_flags})
target_include_directories(spice PUBLIC include)
target_link_libraries(spice PUBLIC Threads::Threads)

if(spice_assert_preconditions)
	target_compile_definitions(spice PUBLIC SPICE_ASSERT_PRECONDITIONS)
endif()
if(spice_assert_invariants)
	target_compile_definitions(spice PUBLIC SPICE_ASSERT_INVARIANTS)
endif()